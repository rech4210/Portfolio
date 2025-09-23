#pragma once

#include "CoreMinimal.h"
#include "InventoryDTOs.generated.h"

// Shared Inventory Data Transfer Objects moved from DatabaseModule

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FInventoryItemDTO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 SlotIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FString ItemData; // Serialized JSON or custom data

	FInventoryItemDTO() = default;
	FInventoryItemDTO(const FName& InItemID, int32 InQuantity, int32 InSlotIndex, const FString& InItemData)
		: ItemID(InItemID), Quantity(InQuantity), SlotIndex(InSlotIndex), ItemData(InItemData) {}
};
