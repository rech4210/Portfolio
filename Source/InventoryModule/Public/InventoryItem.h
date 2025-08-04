#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "InventoryItem.generated.h"

UCLASS(BlueprintType)
class INVENTORYMODULE_API UFInventoryItem : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<const UItemDataAsset> ItemData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 Quantity;

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
