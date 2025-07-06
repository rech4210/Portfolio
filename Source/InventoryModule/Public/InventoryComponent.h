// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h" // Use the new separated header
#include "InventoryComponent.generated.h"

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

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	/** The actual list of items in the inventory. Replicated to the owning client. */
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<TObjectPtr<UFInventoryItem>> Items;

	/** Called on the client when the Items array is replicated. */
	UFUNCTION()
	void OnRep_Items();
};