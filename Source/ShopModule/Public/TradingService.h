// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TradingService.generated.h"

class ACharacter;
class AShopSeller;
class UItemDataAsset;

/**
 * Domain Service for handling trading logic between aggregates.
 * This service coordinates transactions between a buyer's inventory, a seller's inventory,
 * and potentially other systems like currency.
 */
UCLASS()
class SHOPMODULE_API UTradingService : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Attempts to perform a purchase transaction.
     * It checks all business rules (e.g., can afford, has stock, has space)
     * and performs the item/currency exchange.
     * @param Buyer The character attempting to buy.
     * @param Shop The shop being bought from.
     * @param ItemData The item to purchase.
     * @param Quantity The amount to purchase.
     * @return True if the transaction was successful.
     */
    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool AttemptToBuyItem(ACharacter* Buyer, const AShopSeller* Shop, const UItemDataAsset* ItemData, int32 Quantity);
    
    // UFUNCTION(BlueprintCallable, Category = "Trading")
    // bool AttemptToSellItem(ACharacter* Seller, AShopManager* Shop, const UItemDataAsset* ItemData, int32 Quantity);
};
