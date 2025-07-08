// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryDomainService.h"
#include "InventoryComponent.h"
#include "InventoryRepository.h"
#include "InventorySubsystem.h"
#include "GameFramework/PlayerState.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Tasks/Task.h"
#include "Async/Async.h"

UInventoryDomainService::UInventoryDomainService()
{
	// Constructor
}

void UInventoryDomainService::Initialize(TScriptInterface<IInventoryRepositoryInterface> Repository)
{
	// If no repository is provided, get it from the subsystem
	if (Repository.GetInterface())
	{
		InventoryRepository = Repository;
	}
	else
	{
		// Get repository from GameInstance subsystem
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (UInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<UInventorySubsystem>())
				{
					InventoryRepository = InventorySubsystem->GetInventoryRepository();
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("InventoryDomainService: Initialized with repository from %s"), 
		Repository.GetInterface() ? TEXT("parameter") : TEXT("subsystem"));
}

UE::Tasks::TTask<bool> UInventoryDomainService::AddItemToInventory(APlayerState* PlayerState, const FInventoryItemDTO& Item)
{
	if (!PlayerState || !InventoryRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepoInterface = PlayerState->GetGameInstance()->
		GetSubsystem<UInventorySubsystem>()->GetInventoryRepository();
	
	if (!InventoryComponent || !InventoryRepoInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on PlayerState"));
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("No InventoryComponent or Interface found"));
		});
		
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// Subscribe to domain events if not already subscribed
	SubscribeToDomainEvents(InventoryComponent);

	// 1. Domain validation through Aggregate
	if (!InventoryComponent->CanAddItem(Item))
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Cannot add item - domain rules violation"));
		});
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// 2. Apply change to aggregate first (optimistic update)
	bool bOptimisticUpdateSuccess = InventoryComponent->AddItemDirect(Item);
	if (!bOptimisticUpdateSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to apply optimistic update"));
		});
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// 3. Persist to database through repository
	if (InventoryRepoInterface)
	{
		// Create async task chain using AddNested for proper task dependency
		auto PersistTask = InventoryRepoInterface->AddItemToPlayer(PlayerState, Item);
		
		// Create continuation task with Prerequisites
		auto ContinuationTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, Item, InventoryComponent]() -> bool
		{
			// This will be executed on GameThread after PersistTask completes
			bool bPersistSuccess = false; // PersistTask result will be available here
			
			// We need to check the actual result from the persist task
			// For now, we'll handle success/failure in a different pattern
			return true;
		}, ENamedThreads::GameThread);
		
		// Set up proper task dependency
		ContinuationTask.AddPrerequisites(PersistTask);
		
		// Handle the result in a simpler way using nested tasks
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [PersistTask = MoveTemp(PersistTask), this, PlayerState, Item, InventoryComponent]() mutable -> bool
		{
			bool bSuccess = PersistTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, Item, InventoryComponent, bSuccess]()
			{
				if (bSuccess)
				{
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Add Item"));
				}
				else
				{
					// Rollback optimistic update
					InventoryComponent->RemoveItemDirect(Item.ItemID);
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to persist item to database"));
				}
			}, ENamedThreads::GameThread);
			
			return bSuccess;
		});
	}

	// Fallback for interface-only access
	AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
	{
		OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Repository does not support async operations"));
	});
	return UE::Tasks::MakeCompletedTask<bool>(false);
}

UE::Tasks::TTask<bool> UInventoryDomainService::RemoveItemFromInventory(APlayerState* PlayerState, const FName& ItemID, int32 Quantity)
{
	if (!PlayerState || !InventoryRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepoInterface = PlayerState->GetGameInstance()->
	GetSubsystem<UInventorySubsystem>()->GetInventoryRepository();
	
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on PlayerState"));
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("No InventoryComponent found"));
		});
		
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// Subscribe to domain events if not already subscribed
	SubscribeToDomainEvents(InventoryComponent);

	// 1. Domain validation through Aggregate
	if (!InventoryComponent->CanRemoveItem(ItemID, Quantity))
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Cannot remove item - domain rules violation"));
		});
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// 2. Apply change to aggregate first (optimistic update)
	bool bOptimisticUpdateSuccess = InventoryComponent->RemoveItemDirect(ItemID, Quantity);
	if (!bOptimisticUpdateSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to apply optimistic update"));
		});
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// 3. Persist to database through repository
	if (InventoryRepoInterface)
	{
		// Handle task chain using proper UE::Tasks pattern
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [InventoryRepoInterface, PlayerState, ItemID, Quantity, this, InventoryComponent]() mutable -> bool
		{
			auto PersistTask = InventoryRepoInterface->RemoveItemFromPlayer(PlayerState, ItemID, Quantity);
			bool bSuccess = PersistTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, ItemID, Quantity, InventoryComponent, bSuccess]()
			{
				if (bSuccess)
				{
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Remove Item"));
				}
				else
				{
					// Rollback optimistic update (re-add the item)
					FInventoryItemDTO RollbackItem;
					RollbackItem.ItemID = ItemID;
					RollbackItem.Quantity = Quantity;
					InventoryComponent->AddItemDirect(RollbackItem);
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to persist removal to database"));
				}
			}, ENamedThreads::GameThread);
			
			return bSuccess;
		});
	}

	// Fallback for interface-only access
	AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
	{
		OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Repository does not support async operations"));
	});
	return UE::Tasks::MakeCompletedTask<bool>(false);
}

UE::Tasks::TTask<bool> UInventoryDomainService::LoadInventory(APlayerState* PlayerState)
{
	if (!PlayerState || !InventoryRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepoInterface = PlayerState->GetGameInstance()->
	GetSubsystem<UInventorySubsystem>()->GetInventoryRepository();
	
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// Subscribe to domain events
	SubscribeToDomainEvents(InventoryComponent);

	// Load through repository
	if (InventoryRepoInterface)
	{
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [InventoryRepoInterface, PlayerState, this]() -> bool
		{
			auto LoadTask = InventoryRepoInterface->LoadInventoryForPlayer(PlayerState);
			bool bSuccess = LoadTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, bSuccess]()
			{
				if (bSuccess)
				{
					OnInventoryLoadCompleted.Broadcast(PlayerState);
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Load Inventory"));
				}
				else
				{
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to load inventory from database"));
				}
			}, ENamedThreads::GameThread);
			
			return bSuccess;
		});
	}

	// Fallback for interface-only access
	InventoryRepository->RequestLoadInventoryForPlayer(PlayerState);
	return UE::Tasks::MakeCompletedTask<bool>(true);
}

UE::Tasks::TTask<bool> UInventoryDomainService::SaveInventory(APlayerState* PlayerState)
{
	if (!PlayerState || !InventoryRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	TScriptInterface<IInventoryRepositoryInterface> InventoryRepoInterface = PlayerState->GetGameInstance()->
	GetSubsystem<UInventorySubsystem>()->GetInventoryRepository();
	
	// Save through repository
	if (InventoryRepoInterface)
	{
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [InventoryRepoInterface, PlayerState, this]() -> bool
		{
			auto SaveTask = InventoryRepoInterface->SaveInventoryForPlayer(PlayerState);
			bool bSuccess = SaveTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, bSuccess]()
			{
				if (bSuccess)
				{
					OnInventorySaveCompleted.Broadcast(PlayerState);
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Save Inventory"));
				}
				else
				{
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to save inventory to database"));
				}
			}, ENamedThreads::GameThread);
			
			return bSuccess;
		});
	}

	// Fallback for interface-only access
	InventoryRepository->RequestSaveInventoryForPlayer(PlayerState);
	return UE::Tasks::MakeCompletedTask<bool>(true);
}

void UInventoryDomainService::SubscribeToDomainEvents(UInventoryComponent* InventoryComponent)
{
	if (!InventoryComponent)
	{
		return;
	}

	// Subscribe to domain events from the aggregate
	InventoryComponent->OnInventoryItemAdded.AddUFunction(this, FName("OnDomainItemAdded"));
	InventoryComponent->OnInventoryItemRemoved.AddUFunction(this, FName("OnDomainItemRemoved"));
	InventoryComponent->OnInventoryChanged.AddUFunction(this, FName("OnDomainInventoryChanged"));
}

void UInventoryDomainService::UnsubscribeFromDomainEvents(UInventoryComponent* InventoryComponent)
{
	if (!InventoryComponent)
	{
		return;
	}

	// Unsubscribe from domain events
	InventoryComponent->OnInventoryItemAdded.RemoveAll(this);
	InventoryComponent->OnInventoryItemRemoved.RemoveAll(this);
	InventoryComponent->OnInventoryChanged.RemoveAll(this);
}

void UInventoryDomainService::OnDomainItemAdded(UFInventoryItem* AddedItem)
{
	// Handle domain event - Application Service layer response
	if (AddedItem && AddedItem->ItemData)
	{
		UE_LOG(LogTemp, Log, TEXT("Domain Event Received: Item Added - %s"), *AddedItem->ItemData->GetItemID().ToString());
		// Could trigger other application-level side effects here
	}
}

void UInventoryDomainService::OnDomainItemRemoved(const FName& ItemID, int32 Quantity)
{
	// Handle domain event - Application Service layer response
	UE_LOG(LogTemp, Log, TEXT("Domain Event Received: Item Removed - %s (Quantity: %d)"), *ItemID.ToString(), Quantity);
	// Could trigger other application-level side effects here
}

void UInventoryDomainService::OnDomainInventoryChanged()
{
	// Handle domain event - Could trigger auto-save or other application logic
	UE_LOG(LogTemp, VeryVerbose, TEXT("Domain Event Received: Inventory state changed"));
}
