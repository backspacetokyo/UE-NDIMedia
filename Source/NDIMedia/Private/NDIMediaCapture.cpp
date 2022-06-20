// Fill out your copyright notice in the Description page of Project Settings.


#include "NDIMediaCapture.h"
#include "NDIMediaOutput.h"

#include <chrono>

struct NDIFrameBuffer
{
	NDIlib_video_frame_v2_t frame;
	std::vector<uint8_t> buffer;
};

UNDIMediaCapture::UNDIMediaCapture(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UNDIMediaCapture::HasFinishedProcessing() const
{
	return Super::HasFinishedProcessing();
}

bool UNDIMediaCapture::ValidateMediaOutput() const
{
	UNDIMediaOutput* Output = CastChecked<UNDIMediaOutput>(MediaOutput);
	check(Output);
	return true;
}

bool UNDIMediaCapture::CaptureSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport)
{
	UNDIMediaOutput* Output = CastChecked<UNDIMediaOutput>(MediaOutput);
	OutputPixelFormat = Output->OutputPixelFormat;
	return InitNDI(Output);
}

bool UNDIMediaCapture::CaptureRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget)
{
	return false;
}

bool UNDIMediaCapture::UpdateSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport)
{
	return false;
}

bool UNDIMediaCapture::UpdateRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget)
{
	return false;
}

void UNDIMediaCapture::StopCaptureImpl(bool bAllowPendingFrameToBeProcess)
{
	Super::StopCaptureImpl(bAllowPendingFrameToBeProcess);
	DisposeNDI();
}

void UNDIMediaCapture::OnFrameCaptured_RenderingThread(const FCaptureBaseData& InBaseData,
	TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, void* InBuffer, int32 Width, int32 Height,
	int32 BytesPerRow)
{
	NDIFrameBuffer* FrameBuffer = new NDIFrameBuffer();

	FrameBuffer->frame.frame_rate_N = 60;
	FrameBuffer->frame.frame_rate_D = 1;
	
	if (OutputPixelFormat == ENDIMediaOutputPixelFormat::NDI_PF_P210)
	{
		NDIlib_video_frame_v2_t video_frame_V210;
		video_frame_V210.line_stride_in_bytes = Width * 16;
		video_frame_V210.xres = Width * 6;
		video_frame_V210.yres = Height;
		video_frame_V210.FourCC = NDIlib_FourCC_type_P216;
		video_frame_V210.p_data = (uint8_t*)InBuffer;
	
		NDIlib_video_frame_v2_t& video_frame_V216 = FrameBuffer->frame;
		video_frame_V216.line_stride_in_bytes = video_frame_V210.xres * sizeof(uint16_t) * 4;
	
		FrameBuffer->buffer.resize(video_frame_V216.line_stride_in_bytes * video_frame_V210.yres * sizeof(uint16_t));
		
		video_frame_V216.p_data = &FrameBuffer->buffer[0];
	
		NDIlib_util_V210_to_P216(&video_frame_V210, &video_frame_V216);
	}
	else if (OutputPixelFormat == ENDIMediaOutputPixelFormat::NDI_PF_RGB)
	{
		const uint8_t* ptr = (const uint8_t*)InBuffer;
		FrameBuffer->buffer.assign(ptr, ptr + (BytesPerRow * Height));
		
		NDIlib_video_frame_v2_t& video_frame_rgb = FrameBuffer->frame;
		video_frame_rgb.line_stride_in_bytes = Width * 4;
		video_frame_rgb.xres = Width;
		video_frame_rgb.yres = Height;
		video_frame_rgb.FourCC = NDIlib_FourCC_type_BGRX;
		video_frame_rgb.p_data = &FrameBuffer->buffer[0];
	}

	// NDIlib_send_send_video_async_v2(pNDI_send, &FrameBuffer->frame);

	{
		FScopeLock ScopeLock(&RenderThreadCriticalSection);
		FrameBuffers.push_back(FrameBuffer);
	}
}

bool UNDIMediaCapture::InitNDI(UNDIMediaOutput* Output)
{
	FScopeLock ScopeLock(&RenderThreadCriticalSection);

	NDIlib_send_create_t settings;
	settings.p_ndi_name = TCHAR_TO_ANSI(*Output->SourceName);
	pNDI_send = NDIlib_send_create(&settings);

	if (!pNDI_send)
	{
		SetState(EMediaCaptureState::Error);
		return false;
	}

	SetState(EMediaCaptureState::Capturing);

	///
	
	const int N = 4;
	FrameBuffers.resize(N);

	for (int i = 0; i < FrameBuffers.size(); i++)
		FrameBuffers[i] = new NDIFrameBuffer();

	NDISendThreadRunning = true;
	NDISendThread = std::thread([this]()
	{
		while (this->NDISendThreadRunning)
		{
			bool has_queue = false;

			{
				FScopeLock ScopeLock(&this->RenderThreadCriticalSection);
				has_queue = this->FrameBuffers.size() > 0;
			}

			if (has_queue)
			{
				NDIFrameBuffer* FrameBuffer = nullptr;
				{
					FScopeLock ScopeLock(&this->RenderThreadCriticalSection);
					FrameBuffer = this->FrameBuffers.front();
					this->FrameBuffers.pop_front();
				}

				NDIlib_send_send_video_v2(pNDI_send, &FrameBuffer->frame);
				
				delete FrameBuffer;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	});
	
	return true;
}

bool UNDIMediaCapture::DisposeNDI()
{
	FScopeLock ScopeLock(&RenderThreadCriticalSection);

	NDISendThreadRunning = false;
	NDISendThread.join();
	
	if (pNDI_send)
	{
		NDIlib_send_destroy(pNDI_send);
		pNDI_send = nullptr;
	}

	for (int i = 0; i < FrameBuffers.size(); i++)
		delete FrameBuffers[i];

	FrameBuffers.clear();
	
	return true;
}
