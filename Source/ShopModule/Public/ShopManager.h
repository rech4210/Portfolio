// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShopManager.generated.h"

class UInventoryComponent;
class UItemDataAsset;
class ACharacter;

/**
 * An actor that represents a shop in the world.
 * This class is responsible for managing the items available for sale
 * and validating purchase/sale requests from players.
 * The actual transaction logic (granting/taking items and currency) is handled by GameplayAbilities.
 */
UCLASS()
class SHOPMODULE_API AShopManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AShopManager();

protected:
	/** The list of items this shop has for sale. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop")
	TArray<UItemDataAsset*> ItemsForSale;

public:
	/**
	 * Checks if a character can buy a specific item from this shop.
	 * This should be called from a server-side Gameplay Ability before the transaction.
	 * @param Buyer The character attempting to buy the item.
	 * @param ItemToBuy The specific data asset of the item to purchase.
	 * @return True if the item is for sale and the buyer can purchase it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool CanBuyItem(ACharacter* Buyer, const UItemDataAsset* ItemToBuy) const;

	/**
	 * Checks if a character can sell a specific item to this shop.
	 * For now, we assume the shop buys any item, but this provides a hook for future logic
	 * (e.g., only buying specific types of items).
	 * @param Seller The character attempting to sell the item.
	 * @param ItemToSell The specific data asset of the item to sell.
	 * @return True if the shop is willing to buy this item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool CanSellItem(ACharacter* Seller, const UItemDataAsset* ItemToSell) const;

	/**
	 * Gets the list of items available in this shop.
	 * @return Array of items for sale.
	 */
	UFUNCTION(BlueprintPure, Category = "Shop")
	const TArray<UItemDataAsset*> GetItemsForSale() { return ItemsForSale; }
}; 