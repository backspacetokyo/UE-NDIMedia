// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IMediaPlayerFactory.h"

#define PROCESSINGNDILIB_STATIC

#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Lib.cplusplus.h>

class FNDIMediaModule : public IModuleInterface, public IMediaPlayerFactory
{
	TArray<FString> SupportedPlatforms;
	TArray<FString> SupportedUriSchemes;

	NDIlib_find_instance_t pNDI_find = nullptr;
	
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IMediaPlayerFactory implementation */
	virtual bool CanPlayUrl(const FString& Url, const IMediaOptions* /*Options*/, TArray<FText>* /*OutWarnings*/,
							TArray<FText>* OutErrors) const override;

	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override;

	virtual FText GetDisplayName() const override;
	virtual FName GetPlayerName() const override;
	virtual FGuid GetPlayerPluginGUID() const override;
	virtual const TArray<FString>& GetSupportedPlatforms() const override;
	virtual bool SupportsFeature(EMediaFeature Feature) const override;

public:

	NDIlib_find_instance_t GetNDIFind() const;
};
