
#pragma once

#include "CoreMinimal.h"
#include "InventoryDomain.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "Interface/PlayerIdentityInterface.h"
#include "InventoryDomainService.generated.h"

class UInventoryComponent;
class IInventoryRepositoryInterface;
struct FInventoryItemDTO;

// Domain Events (발행: Aggregate/Component)
// DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemAdded, APlayerState* /* PlayerState */, const FInventoryItemDTO& /* Item */);
// DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInventoryItemRemoved, APlayerState* /* PlayerState */, const FName& /* ItemID */, int32 /* Quantity */);

// Application Events (발행: DomainService)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationSucceeded, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */, const FString& /* Operation */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationFailed, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */, const FString& /* Reason */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryLoadCompleted, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventorySaveCompleted, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */);

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
	UE::Tasks::TTask<void> AddItemToInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryItemDTO& Item);

	/**
	 * Domain Service: Remove item from player's inventory with full business logic
	 * @param PlayerState Target player
	 * @param ItemID Item to remove
	 * @param Quantity How many to remove
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<void> RemoveItemFromInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FName& ItemID, int32 Quantity);

	/**
	 * Domain Service: Load player's inventory from persistence
	 * @param PlayerState Target player
	 * @return Task that completes when loading finishes
	 */
	UE::Tasks::TTask<void> LoadInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);

	/**
	 * Domain Service: Save player's current inventory state
	 * @param PlayerState Target player
	 * @param InventoryData
	 * @return Task that completes when save finishes
	 */
	UE::Tasks::TTask<void> SaveInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryDomain& InventoryData);

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
