// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "InventoryDomainService.generated.h"

class UInventoryComponent;
class IInventoryRepositoryInterface;
class APlayerState;
struct FInventoryItemDTO;

// Domain Events (발행: Aggregate/Component)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemAdded, APlayerState* /* PlayerState */, const FInventoryItemDTO& /* Item */);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInventoryItemRemoved, APlayerState* /* PlayerState */, const FName& /* ItemID */, int32 /* Quantity */);

// Application Events (발행: DomainService)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationSucceeded, APlayerState* /* PlayerState */, const FString& /* Operation */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationFailed, APlayerState* /* PlayerState */, const FString& /* Reason */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryLoadCompleted, APlayerState* /* PlayerState */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventorySaveCompleted, APlayerState* /* PlayerState */);

/**
 * Domain Service for Inventory operations
 * Handles business logic orchestration and coordinates between Aggregates and Repositories
 */
UCLASS(BlueprintType)
class INVENTORYMODULE_API UInventoryDomainService : public UObject
{
	GENERATED_BODY()

public:
	UInventoryDomainService();

	/**
	 * Initialize the domain service with repository dependency
	 * @param Repository The repository interface to use for persistence
	 */
	void Initialize(TScriptInterface<IInventoryRepositoryInterface> Repository);

	/**
	 * Domain Service: Add item to player's inventory with full business logic
	 * @param PlayerState Target player
	 * @param Item Item to add
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<bool> AddItemToInventory(APlayerState* PlayerState, const FInventoryItemDTO& Item);

	/**
	 * Domain Service: Remove item from player's inventory with full business logic
	 * @param PlayerState Target player
	 * @param ItemID Item to remove
	 * @param Quantity How many to remove
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<bool> RemoveItemFromInventory(APlayerState* PlayerState, const FName& ItemID, int32 Quantity);

	/**
	 * Domain Service: Load player's inventory from persistence
	 * @param PlayerState Target player
	 * @return Task that completes when loading finishes
	 */
	UE::Tasks::TTask<bool> LoadInventory(APlayerState* PlayerState);

	/**
	 * Domain Service: Save player's current inventory state
	 * @param PlayerState Target player
	 * @return Task that completes when save finishes
	 */
	UE::Tasks::TTask<bool> SaveInventory(APlayerState* PlayerState);

	// Application Events
	FOnInventoryOperationSucceeded OnInventoryOperationSucceeded;
	FOnInventoryOperationFailed OnInventoryOperationFailed;
	FOnInventoryLoadCompleted OnInventoryLoadCompleted;
	FOnInventorySaveCompleted OnInventorySaveCompleted;

private:
	// Repository dependency (injected)
	UPROPERTY()
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepository;

	/**
	 * Subscribe to domain events from InventoryComponent
	 */
	void SubscribeToDomainEvents(UInventoryComponent* InventoryComponent);

	/**
	 * Unsubscribe from domain events
	 */
	void UnsubscribeFromDomainEvents(UInventoryComponent* InventoryComponent);

	// Domain event handlers
	UFUNCTION()
	void OnDomainItemAdded(UFInventoryItem* AddedItem);

	UFUNCTION()
	void OnDomainItemRemoved(const FName& ItemID, int32 Quantity);

	UFUNCTION()
	void OnDomainInventoryChanged();
};
