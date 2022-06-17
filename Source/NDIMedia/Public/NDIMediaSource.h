// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMediaSource.h"

#include "NDIMediaSource.generated.h"

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="NDI Media Source"), HideCategories=("Platforms"))
class NDIMEDIA_API UNDIMediaSource
	: public UBaseMediaSource
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source")
	FString SourceName = FString("Test Pattern");

	virtual bool Validate() const override { return true; }
	virtual FString GetUrl() const override;
};
