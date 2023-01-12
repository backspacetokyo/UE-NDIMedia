// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaIOCorePlayerBase.h"

#include "NDIMediaSample.h"
#include "NDIMediaSource.h"

#define PROCESSINGNDILIB_STATIC
#include <Processing.NDI.Lib.h>

class FNDIMediaModule;
class FNDIMediaPlayer;

DECLARE_DELEGATE_OneParam(FNDIMediaPlayerFrameMetadataReceivedDelegate, FString)

class FNDIMediaPlayerThread : public FRunnable
{
public:

	struct Arguments
	{
		FNDIMediaPlayer* Player;
		NDIlib_recv_instance_t pNDI_recv;
	};

	FNDIMediaPlayerThread(const Arguments& args);
	virtual ~FNDIMediaPlayerThread() override;

public:

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

public:

	const Arguments args;
	
	FRunnableThread* Thread = nullptr;
	bool bIsRunning = false;

	NDIlib_recv_instance_t pNDI_recv = nullptr;
};

class NDIMEDIA_API FNDIMediaPlayer
	: public FMediaIOCorePlayerBase
{
	using Super = FMediaIOCorePlayerBase;
	friend class FNDIMediaPlayerThread;
	
	FNDIMediaModule* Module;

	NDIlib_recv_performance_t p_total, p_dropped;

	FNDIMediaTextureSamplePool* TextureSamplePool;
	FNDIMediaBinarySamplePool* MetadataSamplePool;

	TUniquePtr<FNDIMediaPlayerThread> Thread;

	void OnInputFrameReceived(NDIlib_video_frame_v2_t* video_frame);
	
public:
	
	FNDIMediaPlayer(IMediaEventSink& InEventSink);
	virtual ~FNDIMediaPlayer() override;

	/** FMediaIOCorePlayerBase */
	
	virtual FGuid GetPlayerPluginGUID() const override;

	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual void Close() override;

	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

	virtual FString GetStats() const override;

	virtual bool IsHardwareReady() const override;
	virtual void SetupSampleChannels() override;

#if WITH_EDITOR
	virtual const FSlateBrush* GetDisplayIcon() const override;
#endif

	FNDIMediaPlayerFrameMetadataReceivedDelegate FrameMetadataReceivedDelegate;

public:
	
	FFrameRate FrameRate {60, 1};
	void WaitForSync();

private:

	int64_t FrameTimeStamp = 0;
	int64_t LastFrameTimeStamp = 0;
};
