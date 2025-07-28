
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
