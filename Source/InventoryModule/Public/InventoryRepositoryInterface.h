// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSharedModule/Public/Interface/BaseRepositoryInterface.h"
#include "UObject/Interface.h"
#include "InventoryItemData.h"
#include "InventoryDomain.h"
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
	
	// Legacy methods for backward compatibility
	// virtual void RequestLoadInventoryForPlayer(APlayerState* PlayerState) = 0;
	// virtual void RequestSaveInventoryForPlayer(APlayerState* PlayerState) = 0;
	//
	/**
	 * Pure repository methods - no engine dependencies
	 * Repository should only handle persistence, not domain logic
	 */
	
	/**
	 * Load inventory domain data by player ID
	 * @param PlayerId The player ID to load inventory for
	 * @return Task that returns inventory domain data
	 */
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> LoadInventoryByPlayerId(int32 PlayerId) = 0;

	/**
	 * Save inventory domain data
	 * @param InventoryData Pure domain object with inventory data
	 * @return Task that completes when saving finishes
	 */
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> SaveInventoryData(const FInventoryDomain& InventoryData) = 0;

	/**
	 * Add item to inventory by player ID
	 * @param PlayerId The target player ID
	 * @param Item The item to add
	 * @return Task that completes when item is added
	 */
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> AddItemByPlayerId(
		int32 PlayerId, const FInventoryItemDTO& Item) = 0;

	/**
	 * Remove item from inventory by player ID
	 * @param PlayerId The target player ID
	 * @param ItemID The item ID to remove
	 * @param Quantity How many to remove
	 * @return Task that completes when item is removed
	 */
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> RemoveItemByPlayerId(
		int32 PlayerId, const FName& ItemID, int32 Quantity) = 0;

	/**
	 * @deprecated Use pure repository methods instead
	 * These methods mix repository concerns with engine objects
	 */
	// virtual UE::Tasks::TTask<bool> LoadInventoryForPlayer(APlayerState* PlayerState) = 0;
	// virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(APlayerState* PlayerState) = 0;
	// virtual UE::Tasks::TTask<bool> AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item) = 0;
	// virtual UE::Tasks::TTask<bool> RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity) = 0;
};