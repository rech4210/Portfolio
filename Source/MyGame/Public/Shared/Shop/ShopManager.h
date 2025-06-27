// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShopManager.generated.h"

class UInventoryComponent;

/**
 * An actor that represents a shop in the world.
 * Players can interact with this actor to buy and sell items.
 * Interaction and purchases should be handled via GameplayAbilities for security.
 */
UCLASS()
class MYGAME_API AShopManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AShopManager();

protected:
	/** The list of items this shop has for sale. */
	// TODO: Replace FName with your item data type (e.g. FPrimaryAssetId)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TArray<FName> ItemsForSale;

public:	
	/**
	 * Attempts to buy an item for the specified character.
	 * Should be called from a server-side Gameplay Ability.
	 * @param BuyerInventory The inventory component of the character buying the item.
	 * @param ItemID The ID of the item to buy.
	 * @return True if the purchase was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool BuyItem(UInventoryComponent* BuyerInventory, FName ItemID);

	/**
	 * Attempts to sell an item from the specified character's inventory.
	 * Should be called from a server-side Gameplay Ability.
	 * @param SellerInventory The inventory component of the character selling the item.
	 * @param ItemID The ID of the item to sell.
	 * @return True if the sale was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool SellItem(UInventoryComponent* SellerInventory, FName ItemID);
};
