// Fill out your copyright notice in the Description page of Project Settings.


#include "NDIMediaOutputFactory.h"
#include <AssetTypeCategories.h>
#include "NDIMedia/Public/NDIMediaOutput.h"

#define LOCTEXT_NAMESPACE "NDIMediaOutputFactory"

UNDIMediaOutputFactory::UNDIMediaOutputFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->bCreateNew = true;
	this->bEditAfterNew = true;

	this->SupportedClass = UNDIMediaOutput::StaticClass();
}

FText UNDIMediaOutputFactory::GetDisplayName() const
{
	return LOCTEXT("NDIMediaOutputFactoryDisplayName", "NDI Media Output");
}

uint32 UNDIMediaOutputFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}

bool UNDIMediaOutputFactory::ShouldShowInNewMenu() const
{ return true; }

UObject* UNDIMediaOutputFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UNDIMediaOutput>(InParent, InClass, InName, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE