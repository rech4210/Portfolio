// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryRepository.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryDomain.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "InventoryItemData.h"
#include "GameFramework/PlayerState.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Engine/AssetManager.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Experimental/Coroutine/Coroutine.h"
using namespace UE::Tasks;

void UInventoryRepository::Initialize() 
{
	DBManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if (!DBManager) 
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryRepository: DatabaseManager is not available!"));
	}
}

// ============================================================================
// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES
// ============================================================================

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::LoadInventoryByPlayerId(int32 PlayerId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId]() -> FInventoryRepositoryResult
	{
		if (!DBManager)
		{
			return FInventoryRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FInventoryRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}

		// Execute database operation on worker thread
		auto LoadTask = DBManager->LoadInventoryForPlayer(PlayerId);
		TArray<FInventoryItemDTO> LoadedItems = LoadTask.GetResult();

		// Create domain object
		FInventoryDomain InventoryData(PlayerId, LoadedItems);
		return FInventoryRepositoryResult::Success(InventoryData);
	});
}

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::SaveInventoryData(const FInventoryDomain& InventoryData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, InventoryData]() -> FInventoryRepositoryResult
	{
		if (!DBManager)
		{
			return FInventoryRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (!InventoryData.IsValid())
		{
			return FInventoryRepositoryResult::Failure(TEXT("Invalid inventory data"));
		}

		// Execute database operation on worker thread
		auto SaveTask = DBManager->SaveInventoryForPlayer(InventoryData.PlayerId, InventoryData.Items);
		bool bSuccess = SaveTask.GetResult();

		if (bSuccess)
		{
			return FInventoryRepositoryResult::Success(InventoryData);
		}
		else
		{
			return FInventoryRepositoryResult::Failure(TEXT("Failed to save inventory to database"));
		}
	});
}

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::AddItemByPlayerId(
	int32 PlayerId, const FInventoryItemDTO& Item)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, Item]() -> FInventoryRepositoryResult {
		//Use CO_RETURN if c++20 is enabled
		if (!DBManager)
		{
			return FInventoryRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FInventoryRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}
		
		auto bSuccess = DBManager->AddInventoryItem(PlayerId, Item).GetResult();
		if (bSuccess) {
			FInventoryRepositoryResult& Result = LoadInventoryByPlayerId(PlayerId).GetResult();
			return FInventoryRepositoryResult::Success(Result.InventoryData);
		}
		
		// or Use PrerequisitesTask DAG 
		auto PrerequisitesTask = DBManager->AddInventoryItem(PlayerId, Item);
		Launch(UE_SOURCE_LOCATION, [this, PlayerId, PrerequisitesTask]() mutable ->FInventoryRepositoryResult {
			auto Ok = PrerequisitesTask.GetResult();
			if (Ok) {
				TTask<FInventoryRepositoryResult> Result = LoadInventoryByPlayerId(PlayerId);
				return FInventoryRepositoryResult::Success(Result.GetResult().InventoryData);
			}
			else {
				return FInventoryRepositoryResult::Failure(TEXT("Failed to load inventory"));
			}
		},PrerequisitesTask);
		
		return FInventoryRepositoryResult::Failure(TEXT("Failed to add inventory item"));
	});
}

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::RemoveItemByPlayerId(
	int32 PlayerId, const FName& ItemID, int32 Quantity)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, ItemID, Quantity]() -> FInventoryRepositoryResult
	{
		if (!DBManager)
		{
			return FInventoryRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FInventoryRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}

		// Execute database operation on worker thread
		auto RemoveTask = DBManager->RemoveInventoryItem(PlayerId, ItemID, Quantity);
		bool bSuccess = RemoveTask.GetResult();

		if (bSuccess)
		{
			// Return updated inventory data
			auto ReloadTask = LoadInventoryByPlayerId(PlayerId);
			return ReloadTask.GetResult();
		}
		return FInventoryRepositoryResult::Failure(TEXT("Failed to remove item from database"));
	});
}



// ============================================================================
// DEPRECATED METHODS - ENGINE OBJECT DEPENDENCIES
// ============================================================================
//
// void UInventoryRepository::RequestLoadInventoryForPlayer(APlayerState* PlayerState)
// {
// 	// Legacy method - use AsyncTask for non-blocking operation
// 	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, WeakPlayerState = TWeakObjectPtr<APlayerState>(PlayerState)]()
// 	{
// 		if (WeakPlayerState.IsValid())
// 		{
// 			auto LoadTask = LoadInventoryForPlayer(WeakPlayerState.Get());
// 			bool bSuccess = LoadTask.GetResult();
// 			
// 			// Log result on GameThread
// 			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, bSuccess]()
// 			{
// 				if (WeakPlayerState.IsValid())
// 				{
// 					if (bSuccess)
// 					{
// 						UE_LOG(LogTemp, Log, TEXT("Successfully loaded inventory for player %s"), 
// 							*WeakPlayerState->GetPlayerName());
// 					}
// 					else
// 					{
// 						UE_LOG(LogTemp, Error, TEXT("Failed to load inventory for player %s"), 
// 							*WeakPlayerState->GetPlayerName());
// 					}
// 				}
// 			});
// 		}
// 	});
// }
//
// void UInventoryRepository::RequestSaveInventoryForPlayer(APlayerState* PlayerState)
// {
// 	// Legacy method - use AsyncTask for non-blocking operation
// 	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, WeakPlayerState = TWeakObjectPtr<APlayerState>(PlayerState)]()
// 	{
// 		if (WeakPlayerState.IsValid())
// 		{
// 			auto SaveTask = SaveInventoryForPlayer(WeakPlayerState.Get());
// 			bool bSuccess = SaveTask.GetResult();
// 			
// 			// Log result on GameThread
// 			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, bSuccess]()
// 			{
// 				if (WeakPlayerState.IsValid())
// 				{
// 					if (bSuccess)
// 					{
// 						UE_LOG(LogTemp, Log, TEXT("Successfully saved inventory for player %s"), 
// 							*WeakPlayerState->GetPlayerName());
// 					}
// 					else
// 					{
// 						UE_LOG(LogTemp, Error, TEXT("Failed to save inventory for player %s"), 
// 							*WeakPlayerState->GetPlayerName());
// 					}
// 				}
// 			});
// 		}
// 	});
// }
//
// UE::Tasks::TTask<bool> UInventoryRepository::LoadInventoryForPlayer(APlayerState* PlayerState)
// {
// 	if (!PlayerState)
// 	{
// 		return UE::Tasks::MakeCompletedTask<bool>(false);
// 	}
//
// 	// Convert to pure repository call and handle UI updates via AsyncTask
// 	const int32 PlayerId = PlayerState->GetPlayerId();
// 	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;
//
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, WeakPlayerState]() -> bool
// 	{
// 		// Execute pure repository operation on worker thread
// 		auto RepositoryTask = LoadInventoryByPlayerId(PlayerId);
// 		FInventoryRepositoryResult Result = RepositoryTask.GetResult();
//
// 		if (Result.bSuccess)
// 		{
// 			// Update UI components on game thread using AsyncTask
// 			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, Result]()
// 			{
// 				if (WeakPlayerState.IsValid())
// 				{
// 					if (UInventoryComponent* InventoryComponent = WeakPlayerState->FindComponentByClass<UInventoryComponent>())
// 					{
// 						// Convert domain data back to engine objects
// 						TArray<UFInventoryItem*> InventoryItems;
// 						
// 						for (const FInventoryItemDTO& ItemDTO : Result.InventoryData.Items)
// 						{
// 							UFInventoryItem* NewItem = NewObject<UFInventoryItem>(InventoryComponent);
// 							if (NewItem)
// 							{
// 								// Load ItemData from AssetManager
// 								if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
// 								{
// 									FPrimaryAssetId AssetId(TEXT("ItemData"), ItemDTO.ItemID);
// 									if (UItemDataAsset* ItemData = AssetManager->GetPrimaryAssetObject<UItemDataAsset>(AssetId))
// 									{
// 										NewItem->ItemData = ItemData;
// 									}
// 								}
// 								
// 								NewItem->Quantity = ItemDTO.Quantity;
// 								InventoryItems.Add(NewItem);
// 							}
// 						}
//
// 						// Apply loaded items to component
// 						InventoryComponent->Server_SetInventoryItems(InventoryItems);
// 					}
// 				}
// 			});
// 		}
//
// 		return Result.bSuccess;
// 	});
// }
// }
//
// UE::Tasks::TTask<bool> UInventoryRepository::SaveInventoryForPlayer(APlayerState* PlayerState)
// {
// 	if (!PlayerState)
// 	{
// 		return UE::Tasks::MakeCompletedTask<bool>(false);
// 	}
//
// 	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;
// 	const int32 PlayerId = PlayerState->GetPlayerId();
//
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, WeakPlayerState, PlayerId]() -> bool
// 	{
// 		// Get inventory data from component on game thread
// 		TArray<FInventoryItemDTO> ItemDTOs;
// 		bool bDataRetrieved = false;
//
// 		// Use AsyncTask to safely access game thread objects
// 		FEvent* DataRetrievedEvent = FPlatformProcess::GetSynchEventFromPool();
// 		AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, &ItemDTOs, &bDataRetrieved, DataRetrievedEvent]()
// 		{
// 			if (WeakPlayerState.IsValid())
// 			{
// 				if (UInventoryComponent* InventoryComponent = WeakPlayerState->FindComponentByClass<UInventoryComponent>())
// 				{
// 					const TArray<UFInventoryItem*>& Items = InventoryComponent->GetItems();
//
// 					// Convert inventory items to DTOs
// 					for (UFInventoryItem* Item : Items)
// 					{
// 						if (Item && Item->ItemData)
// 						{
// 							FInventoryItemDTO DTO;
// 							DTO.ItemID = Item->ItemData->GetItemID();
// 							DTO.Quantity = Item->Quantity;
// 							
// 							// Serialize additional item data to JSON
// 							TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
// 							JsonObject->SetStringField(TEXT("ItemType"), StaticEnum<EItemType>()->GetNameStringByValue((int64)Item->ItemData->ItemType));
// 							JsonObject->SetNumberField(TEXT("CurrentCount"), Item->ItemData->CurrentCount);
// 							JsonObject->SetNumberField(TEXT("MaxStackCount"), Item->ItemData->MaxStackCount);
// 							JsonObject->SetNumberField(TEXT("Price"), Item->ItemData->Price);
// 							
// 							FString JsonString;
// 							TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
// 							FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
// 							DTO.ItemData = JsonString;
// 							
// 							ItemDTOs.Add(DTO);
// 						}
// 					}
// 					bDataRetrieved = true;
// 				}
// 			}
// 			DataRetrievedEvent->Trigger();
// 		});
//
// 		// Wait for data retrieval to complete
// 		DataRetrievedEvent->Wait();
// 		FPlatformProcess::ReturnSynchEventToPool(DataRetrievedEvent);
//
// 		if (!bDataRetrieved)
// 		{
// 			return false;
// 		}
//
// 		// Create domain object and save via pure repository
// 		FInventoryDomain InventoryData(PlayerId, ItemDTOs);
// 		auto SaveTask = SaveInventoryData(InventoryData);
// 		FInventoryRepositoryResult Result = SaveTask.GetResult();
//
// 		return Result.bSuccess;
// 	});
// }
//
// UE::Tasks::TTask<bool> UInventoryRepository::AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item)
// {
// 	if (!PlayerState)
// 	{
// 		return UE::Tasks::MakeCompletedTask<bool>(false);
// 	}
//
// 	const int32 PlayerId = PlayerState->GetPlayerId();
// 	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;
//
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, Item, WeakPlayerState]() -> bool
// 	{
// 		// Execute pure repository operation on worker thread
// 		auto AddTask = AddItemByPlayerId(PlayerId, Item);
// 		FInventoryRepositoryResult Result = AddTask.GetResult();
//
// 		if (Result.bSuccess)
// 		{
// 			// Update UI components on game thread using AsyncTask
// 			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, Result]()
// 			{
// 				if (WeakPlayerState.IsValid())
// 				{
// 					if (UInventoryComponent* InventoryComponent = WeakPlayerState->FindComponentByClass<UInventoryComponent>())
// 					{
// 						// Reload inventory to reflect changes
// 						// This should trigger domain events properly
// 					}
// 				}
// 			});
// 		}
//
// 		return Result.bSuccess;
// 	});
// }
//
// UE::Tasks::TTask<bool> UInventoryRepository::RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity)
// {
// 	if (!PlayerState)
// 	{
// 		return UE::Tasks::MakeCompletedTask<bool>(false);
// 	}
//
// 	const int32 PlayerId = PlayerState->GetPlayerId();
// 	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;
//
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, ItemID, Quantity, WeakPlayerState]() -> bool
// 	{
// 		// Execute pure repository operation on worker thread
// 		auto RemoveTask = RemoveItemByPlayerId(PlayerId, ItemID, Quantity);
// 		FInventoryRepositoryResult Result = RemoveTask.GetResult();
//
// 		if (Result.bSuccess)
// 		{
// 			// Update UI components on game thread using AsyncTask
// 			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, Result]()
// 			{
// 				if (WeakPlayerState.IsValid())
// 				{
// 					if (UInventoryComponent* InventoryComponent = WeakPlayerState->FindComponentByClass<UInventoryComponent>())
// 					{
// 						// Reload inventory to reflect changes
// 						// This should trigger domain events properly
// 					}
// 				}
// 			});
// 		}
//
// 		return Result.bSuccess;
// 	});
// }
