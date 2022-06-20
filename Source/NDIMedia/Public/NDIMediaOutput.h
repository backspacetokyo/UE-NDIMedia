// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaOutput.h"
#include "MediaIOCoreDefinitions.h"

#include "NDIMediaOutput.generated.h"

UENUM(BlueprintType)
enum class ENDIMediaOutputPixelFormat : uint8
{
	NDI_PF_RGB		UMETA(DisplayName="RGB 8bit"),
	NDI_PF_P210		UMETA(DisplayName="YUV 10bit")
};

UCLASS(BlueprintType, meta=(DisplayName="NDI Media Output"))
class NDIMEDIA_API UNDIMediaOutput
	: public UMediaOutput
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NDI Media")
	FString SourceName = FString("UnrealEngile");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NDI Media")
	FIntPoint OutputSize = FIntPoint(1920, 1080);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NDI Media")
	ENDIMediaOutputPixelFormat OutputPixelFormat = ENDIMediaOutputPixelFormat::NDI_PF_RGB;

	virtual bool Validate(FString& OutFailureReason) const override;

	virtual FIntPoint GetRequestedSize() const override;
	virtual EPixelFormat GetRequestedPixelFormat() const override;
	virtual EMediaCaptureConversionOperation
	GetConversionOperation(EMediaCaptureSourceType InSourceType) const override;

protected:
	virtual UMediaCapture* CreateMediaCaptureImpl() override;

};
