// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSharedModule/Public/Interface/BaseRepositoryInterface.h"
#include "UObject/Interface.h"
#include "InventoryRepositoryInterface.generated.h"

class APlayerState;

// This class does not need to be modified.
UINTERFACE()
class UInventoryRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYMODULE_API IInventoryRepositoryInterface : public IBaseRepositoryInterface {
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual void RequestLoadInventoryForPlayer(APlayerState* PlayerState) = 0;
	virtual void RequestSaveInventoryForPlayer(APlayerState* PlayerState) = 0;
	
	/**
	 * Asynchronously load inventory items for a player
	 * @param PlayerState The player whose inventory to load
	 * @return Task that completes when loading finishes
	 */
	virtual UE::Tasks::TTask<bool> LoadInventoryForPlayer(APlayerState* PlayerState) = 0;

	/**
	 * Asynchronously save inventory items for a player
	 * @param PlayerState The player whose inventory to save
	 * @return Task that completes when saving finishes
	 */
	virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(APlayerState* PlayerState) = 0;

	/**
	 * Add a single item to player's inventory in database
	 * @param PlayerState The target player
	 * @param Item The item to add
	 * @return Task that completes when item is added
	 */
	virtual UE::Tasks::TTask<bool> AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item) = 0;

	/**
	 * Remove a single item from player's inventory in database
	 * @param PlayerState The target player
	 * @param ItemID The item ID to remove
	 * @param Quantity How many to remove
	 * @return Task that completes when item is removed
	 */
	virtual UE::Tasks::TTask<bool> RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity) = 0;

};