// Fill out your copyright notice in the Description page of Project Settings.


#include "NDIMediaSource.h"

UNDIMediaSource::UNDIMediaSource(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString UNDIMediaSource::GetUrl() const
{
	return FString("ndimediain://") + SourceName;
}
