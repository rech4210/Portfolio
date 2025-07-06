// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/ItemDataAsset.h"
#include "InventoryItem.generated.h"

/**
 * Represents a single item in the inventory.
 * This is a data-only class intended for replication and local state management.
 */
UCLASS(BlueprintType)
class INVENTORYMODULE_API UFInventoryItem : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<const UItemDataAsset> ItemData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 Quantity;

    // Helper function to create a new item instance, typically used by the repository.
    static UFInventoryItem* CreateItem(TObjectPtr<const UItemDataAsset> InItemData, int32 InQuantity, UObject* Outer)
    {
        if (!InItemData || InQuantity <= 0 || !Outer)
        {
            return nullptr;
        }
        UFInventoryItem* NewItem = NewObject<UFInventoryItem>(Outer);
        NewItem->ItemData = InItemData;
        NewItem->Quantity = InQuantity;
        return NewItem;
    }
};
