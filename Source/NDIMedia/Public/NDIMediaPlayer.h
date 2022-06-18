// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaIOCorePlayerBase.h"

#include "NDIMediaSample.h"

#define PROCESSINGNDILIB_STATIC
#include <Processing.NDI.Lib.h>

class FNDIMediaModule;

class NDIMEDIA_API FNDIMediaPlayer
	: public FMediaIOCorePlayerBase
{
	using Super = FMediaIOCorePlayerBase;
	
	FNDIMediaModule* Module;
	NDIlib_recv_instance_t pNDI_recv = nullptr;

	FNDIMediaTextureSamplePool* TextureSamplePool;
	FNDIMediaBinarySamplePool* MetadataSamplePool;

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

};
