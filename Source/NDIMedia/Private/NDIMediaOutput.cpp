// Fill out your copyright notice in the Description page of Project Settings.


#include "NDIMediaOutput.h"
#include "NDIMediaCapture.h"

UNDIMediaOutput::UNDIMediaOutput(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UNDIMediaOutput::Validate(FString& OutFailureReason) const
{
	return true;
}

FIntPoint UNDIMediaOutput::GetRequestedSize() const
{
	return OutputSize;
}

EPixelFormat UNDIMediaOutput::GetRequestedPixelFormat() const
{
	EPixelFormat Result = PF_B8G8R8A8;
	
	if (OutputPixelFormat == ENDIMediaOutputPixelFormat::NDI_PF_P210)
		return PF_A2B10G10R10;

	return Result;
}

EMediaCaptureConversionOperation UNDIMediaOutput::GetConversionOperation(EMediaCaptureSourceType InSourceType) const
{
	EMediaCaptureConversionOperation Result = EMediaCaptureConversionOperation::NONE;
	
	if (OutputPixelFormat == ENDIMediaOutputPixelFormat::NDI_PF_P210)
		Result = EMediaCaptureConversionOperation::RGB10_TO_YUVv210_10BIT;

	return Result;
}

UMediaCapture* UNDIMediaOutput::CreateMediaCaptureImpl()
{
	UMediaCapture* Result = NewObject<UNDIMediaCapture>();
	if (Result)
	{
		Result->SetMediaOutput(this);
	}
	return Result;
}
