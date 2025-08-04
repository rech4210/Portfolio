// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GGwaSettingsSaveGame.generated.h"

UCLASS()
class MYGAME_API UGGwaSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UGGwaSettingsSaveGame();

	// TODO: UI 바인딩 - Bind settings UI widgets to these properties.
	
	UPROPERTY(VisibleAnywhere, Category = "Settings|Audio")
	float MasterVolume;

	UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
	int32 GraphicsQuality;

	static const FString SaveSlotName;
}; 