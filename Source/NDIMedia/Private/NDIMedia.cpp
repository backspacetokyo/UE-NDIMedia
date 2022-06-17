// Copyright Epic Games, Inc. All Rights Reserved.

#include "NDIMedia.h"
#include "IMediaModule.h"

#include "NDIMediaPlayer.h"

#define LOCTEXT_NAMESPACE "FNDIMediaModule"

void FNDIMediaModule::StartupModule()
{
	SupportedPlatforms.Add(TEXT("Windows"));

	SupportedUriSchemes.Add(TEXT("ndimediain"));
	SupportedUriSchemes.Add(TEXT("ndimediaout"));

	auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");
	if (MediaModule != nullptr)
	{
		MediaModule->RegisterPlayerFactory(*this);
	}

	check(NDIlib_initialize());
	pNDI_find = NDIlib_find_create_v2();

	UE_LOG(LogTemp, Log, TEXT("NDI version: %s"), *FString(NDIlib_version()));
}

void FNDIMediaModule::ShutdownModule()
{
	NDIlib_find_destroy(pNDI_find);
	NDIlib_destroy();
}

bool FNDIMediaModule::CanPlayUrl(const FString& Url, const IMediaOptions*, TArray<FText>*,
	TArray<FText>* OutErrors) const
{
	FString Scheme;
	FString Location;

	// check scheme
	if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
	{
		if (OutErrors != nullptr)
		{
			OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
		}

		return false;
	}

	if (!SupportedUriSchemes.Contains(Scheme))
	{
		if (OutErrors != nullptr)
		{
			OutErrors->Add(FText::Format(
				LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
		}

		return false;
	}

	return true;
}

TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> FNDIMediaModule::CreatePlayer(IMediaEventSink& EventSink)
{
	return MakeShared<FNDIMediaPlayer, ESPMode::ThreadSafe>(EventSink);
}

FText FNDIMediaModule::GetDisplayName() const
{
	return LOCTEXT("MediaPlayerDisplayName", "NDI Media Interface");
}

FName FNDIMediaModule::GetPlayerName() const
{
	static FName PlayerName(TEXT("NDIMedia"));
	return PlayerName;
}

FGuid FNDIMediaModule::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0xdd39a98d,
								  0x2e60507c,
								  0x0815829f,
								  0x7e2f891f);
	return PlayerPluginGUID;
}

const TArray<FString>& FNDIMediaModule::GetSupportedPlatforms() const
{
	return SupportedPlatforms;
}

bool FNDIMediaModule::SupportsFeature(EMediaFeature Feature) const
{
	return (Feature == EMediaFeature::VideoSamples)
		|| (Feature == EMediaFeature::VideoTracks)
		|| (Feature == EMediaFeature::MetadataTracks);
}

NDIlib_find_instance_t FNDIMediaModule::GetNDIFind() const
{
	return pNDI_find;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNDIMediaModule, NDIMedia)