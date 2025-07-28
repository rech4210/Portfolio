// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataAsset.h"
#include "InventoryItemData.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UItemDataAsset> ItemDataAsset;

	UPROPERTY()
	int32 Quantity;
};
