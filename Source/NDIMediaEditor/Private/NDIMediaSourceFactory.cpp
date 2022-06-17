// Fill out your copyright notice in the Description page of Project Settings.


#include "NDIMediaSourceFactory.h"

#include <AssetTypeCategories.h>

#include "NDIMedia/Public/NDIMediaSource.h"

#define LOCTEXT_NAMESPACE "NDIMediaSourceFactory"

UNDIMediaSourceFactory::UNDIMediaSourceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->bCreateNew = true;
	this->bEditAfterNew = true;

	this->SupportedClass = UNDIMediaSource::StaticClass();
}

FText UNDIMediaSourceFactory::GetDisplayName() const
{
	return LOCTEXT("NDIMediaSourceFactoryDisplayName", "NDI Media Source");
}

uint32 UNDIMediaSourceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}

bool UNDIMediaSourceFactory::ShouldShowInNewMenu() const
{
	return true;
}

UObject* UNDIMediaSourceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                                  UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UNDIMediaSource>(InParent, InClass, InName, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE