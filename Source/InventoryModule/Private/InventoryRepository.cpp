// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryRepository.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryItemData.h"
#include "GameFramework/PlayerState.h"

void UInventoryRepository::LoadInventoryForPlayer(APlayerState* PlayerState)
{
	if (!PlayerState || !PlayerState->HasAuthority())
	{
		return;
	}

	// In a real implementation, this would be an async call to a database or web service.
	// For this example, we'll create some dummy data.
	TArray<FInventoryItemData> LoadedItems;
	// TODO: Replace with actual data loading.
	// FInventoryItemData Item1;
	// Item1.ItemDataAsset = TSoftObjectPtr<UItemDataAsset>(FSoftObjectPath(TEXT("ItemDataAsset'/Game/Data/Items/MyItem.MyItem'")));
	// Item1.Quantity = 10;
	// LoadedItems.Add(Item1);

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	if (InventoryComponent)
	{
		Server_ApplyInventoryData(InventoryComponent, LoadedItems);
	}
}

void UInventoryRepository::SaveInventoryForPlayer(APlayerState* PlayerState)
{
	if (!PlayerState || !PlayerState->HasAuthority())
	{
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	if (InventoryComponent)
	{
		const TArray<UFInventoryItem*>& ItemsToSave = InventoryComponent->GetItems();
		// TODO: Convert UFInventoryItem array to a savable format and send to database.
	}
}

void UInventoryRepository::Server_ApplyInventoryData(UInventoryComponent* InventoryComponent, const TArray<FInventoryItemData>& LoadedItems)
{
	if (!InventoryComponent || !InventoryComponent->GetOwner()->HasAuthority())
	{
		return;
	}

	TArray<UFInventoryItem*> NewItems;
	for (const FInventoryItemData& ItemData : LoadedItems)
	{
		if (UItemDataAsset* ItemAsset = ItemData.ItemDataAsset.LoadSynchronous())
		{
			UFInventoryItem* NewItem = UFInventoryItem::CreateItem(ItemAsset, ItemData.Quantity, InventoryComponent);
			NewItems.Add(NewItem);
		}
	}

	InventoryComponent->Server_SetInventoryItems(NewItems);
}

void UInventoryRepository::Client_OnInventoryUpdated(UInventoryComponent* InventoryComponent)
{
	// This is where you would put any client-side logic that needs to happen after the inventory is updated.
	// For example, you might want to update a local cache or trigger some UI event.
	// For now, we'll just log a message.
	UE_LOG(LogTemp, Log, TEXT("Inventory updated on client for %s"), *InventoryComponent->GetOwner()->GetName());
}