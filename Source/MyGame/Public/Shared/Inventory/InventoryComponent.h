// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "InventoryComponent.generated.h"

/**
 * Represents a single item in the inventory.
 */
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	// TODO: Replace with your item data definition (e.g., FPrimaryAssetId for a DataAsset)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Quantity;

    // Add other properties like instance-specific data
};


/**
 * A component for managing a character's inventory. Designed to be added to PlayerState.
 * The inventory is server-authoritative and replicates to the owning client.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// TODO: UI 바인딩 - This delegate should be bound by the inventory UI to update on changes.
	// FOnInventoryChanged OnInventoryChanged;

	/** Adds an item to the inventory. Should only be called on the server. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(const FInventoryItem& Item);

	/** Removes an item from the inventory. Should only be called on the server. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(FName ItemID, int32 Quantity);

protected:
	/** The actual list of items in the inventory. */
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<FInventoryItem> Items;

	/** Called on the client when the Items array is replicated. */
	UFUNCTION()
	void OnRep_Items();
}; 