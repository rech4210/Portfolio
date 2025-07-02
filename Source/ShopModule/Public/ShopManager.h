// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShopManager.generated.h"

class UInventoryComponent;
class UItemDataAsset;
class ACharacter;

/**
 * An actor that represents a shop in the world. (Shop Aggregate Root)
 * This class is responsible for managing the shop's own inventory.
 * The actual transaction logic is handled by the UTradingService.
 */
UCLASS()
class SHOPMODULE_API AShopManager : public AActor
{
	GENERATED_BODY()
	
protected:
	/** The shop's own inventory component to manage its stock. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> ShopInventory;

public:	
	AShopManager();

	/**
	 * Checks if the shop has a specific item in stock.
	 * @param ItemToBuy The data asset of the item to check.
	 * @param Quantity The amount to check for.
	 * @return True if the shop has enough of the item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool HasItemForSale(const UItemDataAsset* ItemToBuy, int32 Quantity = 1) const;

	/**
	 * Gets the inventory component of this shop.
	 * @return The shop's inventory component.
	 */
	UFUNCTION(BlueprintPure, Category = "Shop")
	UInventoryComponent* GetShopInventoryComponent() const { return ShopInventory; }
}; 