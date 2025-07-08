// Copyright Epic Games, Inc. All Rights Reserved.

#include "TradingService.h"
#include "ShopSeller.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

bool UTradingService::AttemptToBuyItem(ACharacter* Buyer, const AShopSeller* Shop, const UItemDataAsset* ItemData, int32 Quantity)
{
    if (!Buyer || !Shop || !ItemData || Quantity <= 0)
    {
        return false;
    }

    UInventoryComponent* BuyerInventory = Buyer->GetPlayerState()->FindComponentByClass<UInventoryComponent>();
    UShopComponent* ShopComponent = Shop->GetShopComponent();
    UAbilitySystemComponent* BuyerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Buyer);

    if (!BuyerInventory || !ShopComponent || !BuyerASC)
    {
        return false;
    }

    // 1. Check if the shop has the item in stock
    // if (!ShopComponent->HasItemForSale(ItemData, Quantity))
    // {
    //     // TODO: Send feedback to player (e.g., via a GameplayTag event)
    //     return false;
    // }

    // 2. Check if the player can afford the item
    const float TotalCost = ItemData->Price * Quantity;
    // if (ItemData->CostGE && !BuyerASC->CanAffordAttributeChange(ItemData->CostGE, TotalCost))
    // {
    //     // TODO: Send feedback
    //     return false;
    // }

    // 3. Check if the player has inventory space
    // if (!BuyerInventory->HasEnoughSpace(ItemData, Quantity))
    // {
    //     // TODO: Send feedback
    //     return false;
    // }

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
    else {
        return false;
    }

    // 5. Transfer the item
    //get item id
    // 영속화 계층에서 수행할 것.
    // GA가 끝난 후, DB 영속화 이후 실행되도록 Delegate를 활용해도 좋을듯.
    // ShopComponent->UpdateItemStock(ItemData->ItemID, Quantity);
    // BuyerInventory->AddItem(ItemData->GetClass(), Quantity);
    return true;
}
