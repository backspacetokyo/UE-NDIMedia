// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "NDIMediaOutputFactory.generated.h"

UCLASS(hidecategories=Object)
class NDIMEDIAEDITOR_API UNDIMediaOutputFactory
	: public UFactory
{
	GENERATED_UCLASS_BODY()
public:

	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;

	virtual bool ShouldShowInNewMenu() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	                                  UObject* Context, FFeedbackContext* Warn) override;

};
