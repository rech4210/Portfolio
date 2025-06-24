// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GGwaSettingsSaveGame.generated.h"

/**
 * SaveGame object for storing local player settings.
 */
UCLASS()
class MYGAME_API UGGwaSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UGGwaSettingsSaveGame();

	// TODO: UI 바인딩 - Bind settings UI widgets to these properties.
	
	/** The master volume level. */
	UPROPERTY(VisibleAnywhere, Category = "Settings|Audio")
	float MasterVolume;

	/** The graphics quality level (0=low, 1=medium, 2=high, etc.). */
	UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
	int32 GraphicsQuality;

	/** The name of the slot used for saving/loading. */
	static const FString SaveSlotName;
}; 