// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClientPluginBPLibrary.h"
#include "ClientPlugin.h"

UClientPluginBPLibrary::UClientPluginBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UClientPluginBPLibrary::ClientPluginSampleFunction(float Param)
{
	return -1;
}

