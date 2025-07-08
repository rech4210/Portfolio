// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "InventorySubsystem.h"
#include "GameFramework/PlayerState.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Engine/AssetManager.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Only replicate the inventory to the owning client
	DOREPLIFETIME_CONDITION(UInventoryComponent, Items, COND_OwnerOnly);
}

void UInventoryComponent::Server_SetInventoryItems(const TArray<UFInventoryItem*>& InItems)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Items.Empty();
		for (UFInventoryItem* Item : InItems)
		{
			if (Item)
			{
				// The component now takes ownership of the item objects.
				Items.Add(Item);
			}
		}
		// For the server, we can directly call the OnRep function or the delegate.
		// This ensures server-side logic that depends on the inventory state is also executed.
		OnRep_Items();
	}
}

bool UInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	FReplicationFlags* RepFlags) {
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UFInventoryItem* Item : Items)
	{
		if (Item)
		{
			bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
		}
	}
	return bWroteSomething;
}

void UInventoryComponent::OnRep_Items()
{
	// This is called on the client when the 'Items' array is replicated.
	// We notify the subsystem to handle any client-side logic.
	if (GetOwner())
	{
		if (UInventorySubsystem* InventorySubsystem = GetOwner()->GetGameInstance()->GetSubsystem<UInventorySubsystem>())
		{
			InventorySubsystem->Client_OnInventoryUpdated(this);
		}
	}

	// Broadcast the delegate for UI updates.
	OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::HasItem(const FName& ItemID) const
{
	return Items.ContainsByPredicate([ItemID](const UFInventoryItem* Item) {
		return Item && Item->ItemData && Item->ItemData->GetItemID() == ItemID;
	});
}

bool UInventoryComponent::HasEnoughSpace(int32 RequiredSpace) const
{
	// You might want to implement a more sophisticated space check based on your game's requirements
	const int32 MaxInventorySize = 50; // This should probably be configurable
	return (Items.Num() + RequiredSpace) <= MaxInventorySize;
}

bool UInventoryComponent::AddItem(UFInventoryItem* Item)
{
	if (!Item || !HasEnoughSpace())
	{
		return false;
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		Items.Add(Item);
		
		// Domain Event: Broadcast item added
		OnInventoryItemAdded.Broadcast(Item);
		OnInventoryChanged.Broadcast();
		
		OnRep_Items();
		return true;
	}

	return false;
}

bool UInventoryComponent::RemoveItem(const FName& ItemID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		int32 Index = Items.IndexOfByPredicate([ItemID](const UFInventoryItem* Item) {
			return Item && Item->ItemData && Item->ItemData->GetItemID() == ItemID;
		});

		if (Index != INDEX_NONE)
		{
			UFInventoryItem* RemovedItem = Items[Index];
			int32 RemovedQuantity = RemovedItem ? RemovedItem->Quantity : 1;
			
			Items.RemoveAt(Index);
			
			// Domain Event: Broadcast item removed
			OnInventoryItemRemoved.Broadcast(ItemID, RemovedQuantity);
			OnInventoryChanged.Broadcast();
			
			OnRep_Items();
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::CanAddItem(const FInventoryItemDTO& Item) const
{
	// Domain Rule: Validate item quantity
	if (Item.Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid quantity for item %s: %d"), *Item.ItemID.ToString(), Item.Quantity);
		return false;
	}

	// Domain Rule: Check if inventory has enough space
	if (!HasEnoughSpace(1))
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory is full, cannot add item %s"), *Item.ItemID.ToString());
		return false;
	}

	// Domain Rule: Check item validity (could be extended)
	if (Item.ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ItemID"));
		return false;
	}

	return true;
}

bool UInventoryComponent::CanRemoveItem(const FName& ItemID, int32 Quantity) const
{
	// Domain Rule: Validate removal quantity
	if (Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid removal quantity for item %s: %d"), *ItemID.ToString(), Quantity);
		return false;
	}

	// Domain Rule: Check if player has the item
	if (!HasItem(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player does not have item %s"), *ItemID.ToString());
		return false;
	}

	// Domain Rule: Check if enough quantity exists
	int32 CurrentQuantity = 0;
	for (const UFInventoryItem* Item : Items)
	{
		if (Item && Item->ItemData && Item->ItemData->GetItemID() == ItemID)
		{
			CurrentQuantity += Item->Quantity;
		}
	}

	if (CurrentQuantity < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough quantity for item %s. Has: %d, Requested: %d"), 
			*ItemID.ToString(), CurrentQuantity, Quantity);
		return false;
	}

	return true;
}

bool UInventoryComponent::AddItemDirect(const FInventoryItemDTO& Item)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItemDirect can only be called on server"));
		return false;
	}

	// Create new inventory item from DTO
	UFInventoryItem* NewItem = NewObject<UFInventoryItem>(this);
	if (!NewItem)
	{
		return false;
	}

	// Load ItemData from AssetManager
	if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
	{
		FPrimaryAssetId AssetId(TEXT("ItemData"), Item.ItemID);
		if (UItemDataAsset* ItemData = AssetManager->GetPrimaryAssetObject<UItemDataAsset>(AssetId))
		{
			NewItem->ItemData = ItemData;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not load ItemData for %s"), *Item.ItemID.ToString());
			return false;
		}
	}

	NewItem->Quantity = Item.Quantity;

	// Check if we can stack with existing item
	for (UFInventoryItem* ExistingItem : Items)
	{
		if (ExistingItem && ExistingItem->ItemData && 
			ExistingItem->ItemData->GetItemID() == Item.ItemID &&
			ExistingItem->ItemData->MaxStackCount > ExistingItem->Quantity)
		{
			// Stack with existing item
			int32 CanAdd = FMath::Min(Item.Quantity, ExistingItem->ItemData->MaxStackCount - ExistingItem->Quantity);
			ExistingItem->Quantity += CanAdd;
			
			// Fire domain event
			OnInventoryItemAdded.Broadcast(ExistingItem);
			OnInventoryChanged.Broadcast();
			OnRep_Items();
			
			return true;
		}
	}

	// Add as new item
	Items.Add(NewItem);
	
	// Fire domain event
	OnInventoryItemAdded.Broadcast(NewItem);
	OnInventoryChanged.Broadcast();
	OnRep_Items();
	
	return true;
}

bool UInventoryComponent::RemoveItemDirect(const FName& ItemID, int32 Quantity)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemDirect can only be called on server"));
		return false;
	}

	int32 RemainingToRemove = Quantity;
	
	for (int32 i = Items.Num() - 1; i >= 0 && RemainingToRemove > 0; --i)
	{
		UFInventoryItem* Item = Items[i];
		if (Item && Item->ItemData && Item->ItemData->GetItemID() == ItemID)
		{
			int32 ToRemove = FMath::Min(RemainingToRemove, Item->Quantity);
			Item->Quantity -= ToRemove;
			RemainingToRemove -= ToRemove;
			
			// Fire domain event
			OnInventoryItemRemoved.Broadcast(ItemID, ToRemove);
			
			// Remove item if quantity reaches 0
			if (Item->Quantity <= 0)
			{
				Items.RemoveAt(i);
			}
		}
	}
	
	if (RemainingToRemove == 0)
	{
		OnInventoryChanged.Broadcast();
		OnRep_Items();
		return true;
	}
	
	return false;
}
