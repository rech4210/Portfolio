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

UE::Tasks::TTask<void> UInventoryDomainService::AddItemToInventory(APlayerState* PlayerState, const FInventoryItemDTO& Item)
{
	if (!PlayerState || !InventoryRepository.GetInterface())
	{
		OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Invalid parameters for AddItemToInventory"));
		return UE::Tasks::MakeCompletedTask<void>();
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on PlayerState"));
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("No InventoryComponent found"));
		});
		
		return UE::Tasks::MakeCompletedTask<void>();
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
		return UE::Tasks::MakeCompletedTask<void>();
	}

	// 2. Apply change to aggregate first (optimistic update)
	bool bOptimisticUpdateSuccess = InventoryComponent->AddItemDirect(Item);
	if (!bOptimisticUpdateSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to apply optimistic update"));
		});
		return UE::Tasks::MakeCompletedTask<void>();
	}

	// 3. Persist to database through repository
	if (UInventoryRepository* ConcreteRepo = Cast<UInventoryRepository>(InventoryRepository.GetObject()))
	{
		const int32 PlayerId = PlayerState->GetPlayerId();
		
		// Execute add and reload asynchronously
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, ConcreteRepo, PlayerId, Item, PlayerState, InventoryComponent]()
		{
			UE::Tasks::TTask<FInventoryRepositoryResult> RepoTask = ConcreteRepo->AddItemByPlayerId(PlayerId, Item);
			FInventoryRepositoryResult Result = RepoTask.GetResult();
			
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Item, InventoryComponent, Result]()
			{
				if (Result.bSuccess)
				{
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Add Item"));
				}
				else
				{
					InventoryComponent->RemoveItemDirect(Item.ItemID);
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to persist item to database"));
				}
			});
		});
		return UE::Tasks::MakeCompletedTask<void>();
	}

	// Fallback for interface-only access
	AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
	{
		OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Repository does not support async operations"));
	});
	return UE::Tasks::MakeCompletedTask<void>();
}

UE::Tasks::TTask<void> UInventoryDomainService::RemoveItemFromInventory(APlayerState* PlayerState, const FName& ItemID, int32 Quantity)
{
	if (!PlayerState || !InventoryRepository.GetInterface() || Quantity <= 0 || ItemID.IsNone())
	{
		OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Invalid parameters for RemoveItemFromInventory"));
		return UE::Tasks::MakeCompletedTask<void>();
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on PlayerState"));
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("No InventoryComponent found"));
		});
		return UE::Tasks::MakeCompletedTask<void>();
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
		return UE::Tasks::MakeCompletedTask<void>();
	}

	// 2. Apply change to aggregate first (optimistic update)
	bool bOptimisticUpdateSuccess = InventoryComponent->RemoveItemDirect(ItemID, Quantity);
	if (!bOptimisticUpdateSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
		{
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to apply optimistic update"));
		});
		return UE::Tasks::MakeCompletedTask<void>();
	}

	// 3. Persist to database through repository
	if (UInventoryRepository* ConcreteRepo = Cast<UInventoryRepository>(InventoryRepository.GetObject()))
	{
		// Handle task chain using proper UE::Tasks pattern
		auto PersistTask = ConcreteRepo->RemoveItemByPlayerId(PlayerState->GetPlayerId(), ItemID, Quantity);
		
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, ItemID, Quantity, InventoryComponent, PersistTask]() mutable -> void
		{
			FInventoryRepositoryResult& Result = PersistTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, ItemID, Quantity, InventoryComponent, Result]()
			{
				if (Result.bSuccess)
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
			});
		}, PersistTask); // Set prerequisite
	}

	AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
	{
		OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Repository does not support async operations"));
	});
	return UE::Tasks::MakeCompletedTask<void>();
}

UE::Tasks::TTask<void> UInventoryDomainService::LoadInventory(APlayerState* PlayerState)
{
	if (!PlayerState || !InventoryRepository.GetInterface())
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]() {
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Invalid parameters for LoadInventory"));
		});
		return UE::Tasks::MakeCompletedTask<void>();
	}

	UInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<UInventoryComponent>();
	
	if (!InventoryComponent)
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState]() {
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("No InventoryComponent found on PlayerState"));
        });
		return UE::Tasks::MakeCompletedTask<void>();
	}

	SubscribeToDomainEvents(InventoryComponent);

	if (UInventoryRepository* ConcreteRepo = Cast<UInventoryRepository>(InventoryRepository.GetObject()))
	{
		UE::Tasks::TTask<FInventoryRepositoryResult> LoadTask = ConcreteRepo->LoadInventoryByPlayerId(PlayerState->GetPlayerId());
		
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, LoadTask]() mutable -> void
		{
			FInventoryRepositoryResult Result = LoadTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Result]()
			{
				if (Result.bSuccess)
				{
					OnInventoryLoadCompleted.Broadcast(PlayerState);
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Load Inventory"));
				}
				else
				{
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to load inventory from database"));
				}
			});
		}, LoadTask); // Set prerequisite
	}

	// Fallback for interface-only access
	// InventoryRepository->LoadInventoryByPlayerId(PlayerState->GetPlayerId());
	return UE::Tasks::MakeCompletedTask<void>();
}

/*
 * 1. 도메인 서비스 라인의 호출은 반환값을 가지지 않도록 한다.
 * 2. 영속 계층의 작업은 WorkerThread로 이어지게 하고, 도메인 서비스 내부에서 GameThread를 호출하여 작업을 마무리한다.
 * 3. Save와 Load 기능을 다시 살펴보기.
 */

UE::Tasks::TTask<void> UInventoryDomainService::SaveInventory(APlayerState* PlayerState, const FInventoryDomain& InventoryData)
{
	if (!InventoryData.IsValid() || !InventoryRepository.GetInterface())
	{
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, InventoryData]() {
			OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Invalid parameters for SaveInventory"));
			if (!InventoryData.IsValid()) {
				OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("InventoryData is not valid"));
			}
		});
		return UE::Tasks::MakeCompletedTask<void>();
	}
	
	// Save through repository
	if (UInventoryRepository* ConcreteRepo = Cast<UInventoryRepository>(InventoryRepository.GetObject()))
	{
		auto SaveTask = ConcreteRepo->SaveInventoryData(InventoryData);
		
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, SaveTask]() mutable -> void
		{
			auto Result = SaveTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Result]()
			{
				if (Result.bSuccess)
				{
					OnInventorySaveCompleted.Broadcast(PlayerState);
					OnInventoryOperationSucceeded.Broadcast(PlayerState, TEXT("Save Inventory"));
				}
				else
				{
					OnInventoryOperationFailed.Broadcast(PlayerState, TEXT("Failed to save inventory to database"));
				}
			});
		}, SaveTask); // Set prerequisite
	}
	return UE::Tasks::MakeCompletedTask<void>();
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


/*deprecated*/


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
