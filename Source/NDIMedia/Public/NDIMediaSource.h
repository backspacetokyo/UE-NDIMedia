// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMediaSource.h"

#include "NDIMediaSource.generated.h"

UENUM(BlueprintType)
enum class ENDIMediaInputPixelFormat : uint8
{
	NDI_PF_RGB		UMETA(DisplayName="RGB 8bit"),
	NDI_PF_P216		UMETA(DisplayName="YUV 16bit"),
	NDI_PF_422		UMETA(DisplayName="YUV 8bit"),
};

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="NDI Media Source"), HideCategories=("Platforms"))
class NDIMEDIA_API UNDIMediaSource
	: public UBaseMediaSource
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source")
	FString SourceName = FString("Test Pattern");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source")
	ENDIMediaInputPixelFormat InputPixelFormat = ENDIMediaInputPixelFormat::NDI_PF_RGB;

	virtual bool Validate() const override { return true; }
	virtual FString GetUrl() const override;
};
