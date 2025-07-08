// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h" // Use the new separated header
#include "InventoryComponent.generated.h"

// Domain Events for InventoryComponent (Aggregate)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryItemAdded, UFInventoryItem* /* AddedItem */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemRemoved, const FName& /* ItemID */, int32 /* Quantity */);
DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**
 * A component for managing a character's inventory. Designed to be added to PlayerState.
 * The inventory is server-authoritative and replicates to the owning client.
 * This component is responsible for holding and replicating the inventory state.
 * All modification logic should go through the InventoryRepository.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYMODULE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called on the server by the repository to update the inventory state. */
	void Server_SetInventoryItems(const TArray<UFInventoryItem*>& InItems);

	/** Provides read-only access to the inventory items. */
	const TArray<UFInventoryItem*>& GetItems() const { return Items; }

	/** Check if the inventory has a specific item */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(const FName& ItemID) const;

	/** Check if the inventory has enough space */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasEnoughSpace(int32 RequiredSpace = 1) const;

	/** Add an item to the inventory (server-side only) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UFInventoryItem* Item);

	/** Remove an item from the inventory (server-side only) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(const FName& ItemID);

	// Domain logic methods for business rules validation and direct manipulation
	/**
	 * Validate if an item can be added (Domain Rule)
	 * @param Item Item to validate
	 * @return True if item can be added
	 */
	bool CanAddItem(const struct FInventoryItemDTO& Item) const;

	/**
	 * Validate if an item can be removed (Domain Rule)
	 * @param ItemID Item ID to validate
	 * @param Quantity Quantity to remove
	 * @return True if item can be removed
	 */
	bool CanRemoveItem(const FName& ItemID, int32 Quantity = 1) const;

	/**
	 * Add item directly to inventory (used by Domain Service for optimistic updates)
	 * @param Item Item to add
	 * @return True if successfully added
	 */
	bool AddItemDirect(const struct FInventoryItemDTO& Item);

	/**
	 * Remove item directly from inventory (used by Domain Service for optimistic updates)
	 * @param ItemID Item ID to remove
	 * @param Quantity Quantity to remove
	 * @return True if successfully removed
	 */
	bool RemoveItemDirect(const FName& ItemID, int32 Quantity = 1);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	// Domain Events (for DDD compliance)
	FOnInventoryItemAdded OnInventoryItemAdded;
	FOnInventoryItemRemoved OnInventoryItemRemoved;
	FOnInventoryChanged OnInventoryChanged;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	/** The actual list of items in the inventory. Replicated to the owning client. */
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<TObjectPtr<UFInventoryItem>> Items;

	/** Called on the client when the Items array is replicated. */
	UFUNCTION()
	void OnRep_Items();
};