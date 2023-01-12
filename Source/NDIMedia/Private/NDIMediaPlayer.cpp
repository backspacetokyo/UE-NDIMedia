// Fill out your copyright notice in the Description page of Project Settings.


#include "NDIMediaPlayer.h"

#include <assert.h>
#include <string>
#include <vector>

#include "Internationalization/Regex.h"

#include "NDIMedia.h"

#include "IMediaEventSink.h"
#include "MediaIOCoreSamples.h"

FNDIMediaPlayerThread::FNDIMediaPlayerThread(const Arguments& args)
	: args(args)
{
	Thread = FRunnableThread::Create( this, TEXT( "FNDIMediaPlayerThread" ) );
}

FNDIMediaPlayerThread::~FNDIMediaPlayerThread()
{
	if ( Thread != nullptr )
	{
		Thread->Kill( true );
		delete Thread;
	}
}

bool FNDIMediaPlayerThread::Init()
{
	pNDI_recv = args.pNDI_recv;
	return true;
}

void FNDIMediaPlayerThread::Exit()
{
	if (pNDI_recv)
	{
		NDIlib_recv_destroy(pNDI_recv);
		pNDI_recv = nullptr;
	}
}

uint32 FNDIMediaPlayerThread::Run()
{
	bIsRunning = true;
	
	while (bIsRunning)
	{
		NDIlib_video_frame_v2_t* video_frame = new NDIlib_video_frame_v2_t();
		const NDIlib_frame_type_e frame_type = NDIlib_recv_capture_v3(pNDI_recv, video_frame,
		                                                              nullptr, nullptr,
		                                                              100);
		
		if (frame_type == NDIlib_frame_type_video)
			args.Player->OnInputFrameReceived(video_frame);

		NDIlib_recv_free_video_v2(pNDI_recv, video_frame);
		delete video_frame;
	}

	return 0;
}

void FNDIMediaPlayerThread::Stop()
{
	bIsRunning = false;
}

void FNDIMediaPlayer::OnInputFrameReceived(NDIlib_video_frame_v2_t* video_frame)
{
	const FTimespan DecodedTime = FTimespan::FromSeconds(GetPlatformSeconds());

	const FFrameRate FrameRate(video_frame->frame_rate_N, video_frame->frame_rate_D);
	const TOptional<FTimecode> DecodedTimecode;

	// metadata
	if (video_frame->p_metadata)
	{
		std::string str = video_frame->p_metadata;
		const auto BinarySample = MetadataSamplePool->AcquireShared();

		BinarySample->Initialize((const uint8_t*)str.c_str(), str.size() + 1,
			DecodedTime, FrameRate, DecodedTimecode);

		Samples->AddMetadata(BinarySample);

		if (FrameMetadataReceivedDelegate.IsBound())
			FrameMetadataReceivedDelegate.Execute(FString(str.c_str()));
	}

	// video
	if (video_frame->FourCC == NDIlib_FourCC_type_BGRA
		|| video_frame->FourCC == NDIlib_FourCC_type_BGRX)
	{
		const auto TextureSample = TextureSamplePool->AcquireShared();

		TextureSample->Initialize(
			video_frame->p_data,
			video_frame->line_stride_in_bytes * video_frame->yres,
			video_frame->line_stride_in_bytes,
			video_frame->xres,
			video_frame->yres,
			EMediaTextureSampleFormat::CharBGRA,
			DecodedTime,
			FrameRate,
			DecodedTimecode,
			true
		);

		Samples->AddVideo(TextureSample);
	}
	else if (video_frame->FourCC == NDIlib_FourCC_type_P216
		|| video_frame->FourCC == NDIlib_FourCC_type_PA16)
	{
		const auto TextureSample = TextureSamplePool->AcquireShared();

		const int line_stride_in_bytes = video_frame->xres * sizeof(uint16_t) * 4;
		
		const int n_pixels = video_frame->xres * video_frame->yres;
		std::vector<uint16_t> buffer(n_pixels * 4);

		const uint16_t* luminance = (const uint16_t*)video_frame->p_data;
		const uint16_t* cbcr = (const uint16_t*)video_frame->p_data + n_pixels;

		uint16_t* out = nullptr;

		if (video_frame->FourCC == NDIlib_FourCC_type_P216)
		{
			out = &buffer[0];
			for (int i = 0; i < n_pixels; i++)
			{
				out[0] = 0xFFFF; // A
				out[1] = *luminance; // Y

				out += 4;
				luminance++;
			}
		}
		else if (video_frame->FourCC == NDIlib_FourCC_type_PA16)
		{
			const uint16_t* alpha = (const uint16_t*)video_frame->p_data + (int)(n_pixels * 2);

			out = &buffer[0];
			for (int i = 0; i < n_pixels; i++)
			{
				out[0] = *alpha; // A
				out[1] = *luminance; // Y

				out += 4;
				luminance++;
				alpha++;
			}
		}

		out = &buffer[0];
		for (int i = 0; i < n_pixels; i += 2)
		{
			out[2] = cbcr[0]; // Cb
			out[3] = cbcr[1]; // Cr
			out += 4;

			out[2] = cbcr[0]; // Cb
			out[3] = cbcr[1]; // Cr
			out += 4;

			cbcr += 2;
		}

		TextureSample->Initialize(
			&buffer[0],
			line_stride_in_bytes * video_frame->yres,
			line_stride_in_bytes,
			video_frame->xres,
			video_frame->yres,
			EMediaTextureSampleFormat::Y416,
			DecodedTime,
			FrameRate,
			DecodedTimecode,
			true
		);
		
		Samples->AddVideo(TextureSample);
	}
	else if (video_frame->FourCC == NDIlib_FourCC_type_UYVY)
	{
		const auto TextureSample = TextureSamplePool->AcquireShared();

		TextureSample->Initialize(
			video_frame->p_data,
			video_frame->line_stride_in_bytes * video_frame->yres,
			video_frame->line_stride_in_bytes,
			video_frame->xres,
			video_frame->yres,
			EMediaTextureSampleFormat::CharUYVY,
			DecodedTime,
			FrameRate,
			DecodedTimecode,
			true
		);
	
		Samples->AddVideo(TextureSample);
	}
	else if (video_frame->FourCC == NDIlib_FourCC_type_UYVA)
	{
		const auto TextureSample = TextureSamplePool->AcquireShared();

		const int n_pixels = video_frame->xres * video_frame->yres;
		const int n_bytes = video_frame->line_stride_in_bytes * video_frame->yres;
		
		std::vector<uint8_t> buffer_ayuv(n_bytes);

		const uint8_t* uyvy = video_frame->p_data;
		const uint8_t* alpha = video_frame->p_data + n_bytes;
		uint8_t* out_ayuv = &buffer_ayuv[0];

		for (int i = 0; i < n_pixels; i += 2)
		{
			out_ayuv[0] = uyvy[1]; // Y
			out_ayuv[1] = uyvy[0]; // U
			out_ayuv[2] = uyvy[2]; // V
			out_ayuv[3] = alpha[0]; // A
		
			out_ayuv += 4;
			uyvy += 4;
			alpha += 2;
		}

		TextureSample->Initialize(
			&buffer_ayuv[0],
			video_frame->line_stride_in_bytes * video_frame->yres,
			video_frame->line_stride_in_bytes,
			video_frame->xres,
			video_frame->yres,
			EMediaTextureSampleFormat::CharAYUV,
			DecodedTime,
			FrameRate,
			DecodedTimecode,
			true
		);

		Samples->AddVideo(TextureSample);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Unsupported pixel format"));
	}
}

FNDIMediaPlayer::FNDIMediaPlayer(IMediaEventSink& InEventSink)
	: Super(InEventSink)
	, TextureSamplePool(new FNDIMediaTextureSamplePool())
	, MetadataSamplePool(new FNDIMediaBinarySamplePool())
{
}

FNDIMediaPlayer::~FNDIMediaPlayer()
{
	delete TextureSamplePool;
	delete MetadataSamplePool;
}

FGuid FNDIMediaPlayer::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0xdd39a98d,
								  0x2e60507c,
								  0x0815829f,
								  0x7e2f891f);
	return PlayerPluginGUID;
}

bool FNDIMediaPlayer::Open(const FString& Url, const IMediaOptions* Options)
{
	Module = (FNDIMediaModule*)FModuleManager::Get().GetModule(
	"NDIMedia");
	NDIlib_find_instance_t pNDI_find = Module->GetNDIFind();

	CurrentState = EMediaState::Preparing;

	if (!pNDI_find)
	{
		return false;
	}

	FString Scheme;
	FString Location;

	// check scheme
	if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
	{
		return false;
	}

	FString SourceName = Location;

	if (Scheme == FString("ndimediain"))
	{
		uint32_t N = 0;
		const NDIlib_source_t* p_source = NDIlib_find_get_current_sources(pNDI_find, &N);

		int source_index = -1;

		for (uint32_t i = 0; i < N; i++)
		{
			FString ndi_name = FString(p_source[i].p_ndi_name);

			FRegexPattern pattern = FRegexPattern(TEXT("(.*) \\((.*)\\)"));
			FRegexMatcher matcher(pattern, ndi_name);

			if (!matcher.FindNext())
				continue;

			const FString machine_name = matcher.GetCaptureGroup(1);
			const FString ndi_source_name = matcher.GetCaptureGroup(2);

			if (SourceName == ndi_source_name)
			{
				source_index = i;
				break;
			}
		}

		if (source_index == -1)
		{
			return false;
		}

		///

		const UNDIMediaSource* Source = static_cast<const UNDIMediaSource*>(Options);
		ENDIMediaInputPixelFormat NDIPixelFormat = Source->InputPixelFormat;

		NDIlib_recv_create_v3_t settings;
		settings.bandwidth = NDIlib_recv_bandwidth_highest;
		settings.allow_video_fields = false;
		settings.source_to_connect_to = p_source[source_index];

		if (NDIPixelFormat == ENDIMediaInputPixelFormat::NDI_PF_RGB)
		{
			settings.color_format = NDIlib_recv_color_format_BGRX_BGRA;
		}
		else if (NDIPixelFormat == ENDIMediaInputPixelFormat::NDI_PF_P216)
		{
			settings.color_format = NDIlib_recv_color_format_best;
		}
		else if (NDIPixelFormat == ENDIMediaInputPixelFormat::NDI_PF_422)
		{
			settings.color_format = NDIlib_recv_color_format_fastest;
		}
		
		auto pNDI_recv = NDIlib_recv_create_v3(&settings);
		assert(pNDI_recv);

		FNDIMediaPlayerThread::Arguments args;
		args.Player = this;
		args.pNDI_recv = pNDI_recv;
		
		Thread = MakeUnique<FNDIMediaPlayerThread>(args);
		
		///

		CurrentState = EMediaState::Playing;
		EventSink.ReceiveMediaEvent(EMediaEvent::MediaConnecting);

		EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
		EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpened);
		EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackResumed);
	}

	SetRate(1);
	
	return true;
}

void FNDIMediaPlayer::Close()
{
	Thread.Reset();
	Super::Close();
}

void FNDIMediaPlayer::TickFetch(FTimespan DeltaTime, FTimespan Timecode)
{
	FMediaIOCorePlayerBase::TickFetch(DeltaTime, Timecode);
}

void FNDIMediaPlayer::TickInput(FTimespan DeltaTime, FTimespan Timecode)
{
	FMediaIOCorePlayerBase::TickInput(DeltaTime, Timecode);

	if (!Thread || CurrentState != EMediaState::Playing)
	{
		return;
	}

	TickTimeManagement();
}

FString FNDIMediaPlayer::GetStats() const
{
	FString Stats;
	Stats += FString::Printf(TEXT("Frames count		: %i\n"), p_total.video_frames);
	Stats += FString::Printf(TEXT("Frames dropped	: %i\n"), p_dropped.video_frames);
	return Stats;
}

bool FNDIMediaPlayer::IsHardwareReady() const
{
	return CurrentState == EMediaState::Playing;
}

void FNDIMediaPlayer::SetupSampleChannels()
{
}

#if WITH_EDITOR
const FSlateBrush* FNDIMediaPlayer::GetDisplayIcon() const
{
	return nullptr;
}
#endif
