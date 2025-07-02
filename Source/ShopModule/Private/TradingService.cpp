// Copyright Epic Games, Inc. All Rights Reserved.

#include "TradingService.h"
#include "ShopManager.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

bool UTradingService::AttemptToBuyItem(ACharacter* Buyer, const AShopManager* Shop, const UItemDataAsset* ItemData, int32 Quantity)
{
    if (!Buyer || !Shop || !ItemData || Quantity <= 0)
    {
        return false;
    }

    UInventoryComponent* BuyerInventory = Buyer->FindComponentByClass<UInventoryComponent>();
    UInventoryComponent* ShopInventory = Shop->GetShopInventoryComponent();
    UAbilitySystemComponent* BuyerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Buyer);

    if (!BuyerInventory || !ShopInventory || !BuyerASC)
    {
        return false;
    }

    // 1. Check if the shop has the item in stock
    if (!Shop->HasItemForSale(ItemData, Quantity))
    {
        // TODO: Send feedback to player (e.g., via a GameplayTag event)
        return false;
    }

    // 2. Check if the player can afford the item
    const float TotalCost = ItemData->Price * Quantity;
    // if (ItemData->CostGE && !BuyerASC->CanAffordAttributeChange(ItemData->CostGE, TotalCost))
    // {
    //     // TODO: Send feedback
    //     return false;
    // }

    // 3. Check if the player has inventory space
    if (!BuyerInventory->HasEnoughSpace(ItemData, Quantity))
    {
        // TODO: Send feedback
        return false;
    }

    // All checks passed, perform the transaction
    
    // 4. Apply the cost GameplayEffect
    // This part would need a more robust implementation to handle dynamic costs.
    // For now, assuming CostGE handles it.
    if(ItemData->CostGE)
    {
         FGameplayEffectContextHandle EffectContext = BuyerASC->MakeEffectContext();
         EffectContext.AddSourceObject(Shop);
         FGameplayEffectSpecHandle SpecHandle = BuyerASC->MakeOutgoingSpec(ItemData->CostGE, 1.0f, EffectContext);
         if(SpecHandle.IsValid())
         {
             BuyerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
         }
    }

    // 5. Transfer the item
    ShopInventory->RemoveItem(ItemData->GetClass(), Quantity);
    BuyerInventory->AddItem(ItemData->GetClass(), Quantity);

    return true;
}