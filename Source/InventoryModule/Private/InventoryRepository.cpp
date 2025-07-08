// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryRepository.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "InventoryItemData.h"
#include "GameFramework/PlayerState.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Engine/AssetManager.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "Json.h"
#include "JsonUtilities.h"

void UInventoryRepository::Initialize() 
{
	DBManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if (!DBManager) 
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryRepository: DatabaseManager is not available!"));
	}
}

UE::Tasks::TTask<bool> UInventoryRepository::LoadInventoryForPlayer(APlayerState* PlayerState)
{
	if (!DBManager || !PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryRepository: No InventoryComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	
	return DBManager->LoadInventoryForPlayer(PlayerId)
		.AddNested(UE_SOURCE_LOCATION, ENamedThreads::GameThread, [this, InventoryComponent](TArray<FInventoryItemDTO> LoadedItems) -> bool
		{
			// Convert DTOs to inventory items on game thread (safe for UObject creation)
			TArray<UFInventoryItem*> InventoryItems;
			
			for (const FInventoryItemDTO& ItemDTO : LoadedItems)
			{
				// Create inventory item from DTO
				UFInventoryItem* NewItem = NewObject<UFInventoryItem>(InventoryComponent);
				if (NewItem)
				{
					// Load ItemData from AssetManager using ItemID
					if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
					{
						// Convert ItemID to Primary Asset Id for loading
						FPrimaryAssetId AssetId(TEXT("ItemData"), ItemDTO.ItemID);
						if (UItemDataAsset* ItemData = AssetManager->GetPrimaryAssetObject<UItemDataAsset>(AssetId))
						{
							NewItem->ItemData = ItemData;
						}
					}
					
					NewItem->Quantity = ItemDTO.Quantity;
					// Deserialize additional JSON data if needed
					// TODO: Parse ItemDTO.ItemData JSON for extended properties
					InventoryItems.Add(NewItem);
				}
			}

			// Apply loaded items to component (already on game thread)
			InventoryComponent->Server_SetInventoryItems(InventoryItems);
			return true;
		});
}

UE::Tasks::TTask<bool> UInventoryRepository::SaveInventoryForPlayer(APlayerState* PlayerState)
{
	if (!DBManager || !PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryRepository: No InventoryComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	const TArray<UFInventoryItem*>& Items = InventoryComponent->GetItems();

	// Convert inventory items to DTOs
	TArray<FInventoryItemDTO> ItemDTOs;
	for (UFInventoryItem* Item : Items)
	{
		if (Item && Item->ItemData)
		{
			FInventoryItemDTO DTO;
			DTO.ItemID = Item->ItemData->GetItemID();
			DTO.Quantity = Item->Quantity;
			
			// Serialize additional item data to JSON (worker thread safe)
			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
			JsonObject->SetStringField(TEXT("ItemType"), StaticEnum<EItemType>()->GetNameStringByValue((int64)Item->ItemData->ItemType));
			JsonObject->SetNumberField(TEXT("CurrentCount"), Item->ItemData->CurrentCount);
			JsonObject->SetNumberField(TEXT("MaxStackCount"), Item->ItemData->MaxStackCount);
			JsonObject->SetNumberField(TEXT("Price"), Item->ItemData->Price);
			
			FString JsonString;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
			FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
			DTO.ItemData = JsonString;
			
			ItemDTOs.Add(DTO);
		}
	}

	return DBManager->SaveInventoryForPlayer(PlayerId, ItemDTOs);
}

UE::Tasks::TTask<bool> UInventoryRepository::AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item)
{
	if (!DBManager || !PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();

	// Remove problematic code that doesn't compile
	// UE::Tasks::FTask TEST;
	// TEST.
	
	// Use proper task chaining without Then method
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, DBManager = this->DBManager, PlayerId, PlayerState, Item]() -> bool
	{
		auto AddTask = DBManager->AddInventoryItem(PlayerId, Item);
		bool bAddSuccess = AddTask.GetResult();
		
		if (bAddSuccess)
		{
			// Create nested task for reload
			auto ReloadTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState]() -> bool
			{
				auto LoadTask = LoadInventoryForPlayer(PlayerState);
				return LoadTask.GetResult();
			}, ENamedThreads::GameThread);
			
			return ReloadTask.GetResult();
		}
		return false;
	});
}

UE::Tasks::TTask<bool> UInventoryRepository::RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity)
{
	if (!DBManager || !PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	
	// Use proper task chaining without Then method
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, DBManager = this->DBManager, PlayerId, PlayerState, ItemID, Quantity]() -> bool
	{
		auto RemoveTask = DBManager->RemoveInventoryItem(PlayerId, ItemID, Quantity);
		bool bRemoveSuccess = RemoveTask.GetResult();
		
		if (bRemoveSuccess)
		{
			// Create nested task for reload
			auto ReloadTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState]() -> bool
			{
				auto LoadTask = LoadInventoryForPlayer(PlayerState);
				return LoadTask.GetResult();
			}, ENamedThreads::GameThread);
			
			return ReloadTask.GetResult();
		}
		return false;
	});
}

void UInventoryRepository::RequestLoadInventoryForPlayer(APlayerState* PlayerState)
{
	// Legacy method - now uses the new async implementation
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState]()
	{
		auto LoadTask = LoadInventoryForPlayer(PlayerState);
		bool bSuccess = LoadTask.GetResult();
		
		// Execute logging on GameThread
		UE::Tasks::Launch(UE_SOURCE_LOCATION, [PlayerState, bSuccess]()
		{
			if (bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("Successfully loaded inventory for player %s"), 
					PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load inventory for player %s"), 
					PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
			}
		}, ENamedThreads::GameThread);
	});
}

void UInventoryRepository::RequestSaveInventoryForPlayer(APlayerState* PlayerState)
{
	// Legacy method - now uses the new async implementation
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState]()
	{
		auto SaveTask = SaveInventoryForPlayer(PlayerState);
		bool bSuccess = SaveTask.GetResult();
		
		// Execute logging on GameThread
		UE::Tasks::Launch(UE_SOURCE_LOCATION, [PlayerState, bSuccess]()
		{
			if (bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("Successfully saved inventory for player %s"), 
					PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save inventory for player %s"), 
					PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
			}
		}, ENamedThreads::GameThread);
	});
}