// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "InventoryRepository.h"
#include "InventoryComponent.h"
#include "InventoryDomainService.h"
#include "GameFramework/PlayerState.h"

void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	
	// Create default repository implementation
	DefaultInventoryRepository = NewObject<UInventoryRepository>(this, TEXT("DefaultInventoryRepository"));
	DefaultInventoryRepository->Initialize();
	
	// Set as default if no other repository is injected
	if (!InventoryRepositoryInterface.GetInterface())
	{
		InventoryRepositoryInterface = DefaultInventoryRepository;
	}

	DomainService = NewObject<UInventoryDomainService>(this);
	DomainService->Initialize(InventoryRepositoryInterface);
	
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Created new InventoryDomainService"));
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Initialized - Repository management only"));
}

void UInventorySubsystem::Deinitialize()
{
	DefaultInventoryRepository = nullptr;
	InventoryRepositoryInterface = nullptr;
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Deinitialized"));
}

TScriptInterface<IInventoryRepositoryInterface> UInventorySubsystem::GetInventoryRepository() const
{
	return InventoryRepositoryInterface;
}

void UInventorySubsystem::SetInventoryRepository(TScriptInterface<IInventoryRepositoryInterface> Repository)
{
	InventoryRepositoryInterface = Repository;
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Repository implementation changed"));
}

UInventoryDomainService* UInventorySubsystem::GetDomainService() {
	return DomainService;
}

// ============================================================================
// Use Case Orchestration - App Layer Responsibilities Only
// ============================================================================

void UInventorySubsystem::RequestAddItemToInventory(APlayerState* PlayerState, const FInventoryItemDTO& Item)
{
	// 1. Network & Authority Validation (App Layer responsibility)
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerState for item addition"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Item addition requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("InventorySubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging (App Layer responsibility)
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting add item transaction - Player: %s, Item: %s, Quantity: %d"), 
		*PlayerState->GetPlayerName(), *Item.ItemID.ToString(), Item.Quantity);

	// 3. Domain Service Call (Delegate business logic)
	DomainService->AddItemToInventory(PlayerState, Item);
}

void UInventorySubsystem::RequestRemoveItemFromInventory(APlayerState* PlayerState, const FName& ItemID, int32 Quantity)
{
	// 1. Network & Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerState for item removal"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Item removal requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("InventorySubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting remove item transaction - Player: %s, Item: %s, Quantity: %d"), 
		*PlayerState->GetPlayerName(), *ItemID.ToString(), Quantity);

	// 3. Domain Service Call
	DomainService->RemoveItemFromInventory(PlayerState, ItemID, Quantity);
}

void UInventorySubsystem::RequestLoadPlayerInventory(APlayerState* PlayerState)
{
	// 1. Network & Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerState for inventory loading"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Client should wait for replicated data, not load from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("InventorySubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting inventory load transaction - Player: %s"), 
		*PlayerState->GetPlayerName());

	// 3. Domain Service Call
	DomainService->LoadInventory(PlayerState);
}

void UInventorySubsystem::RequestSavePlayerInventory(APlayerState* PlayerState, const FInventoryDomain& InventoryData)
{
	// 1. Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerState for inventory saving"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Inventory save requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("InventorySubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting inventory save transaction - Player: %s"), 
		*PlayerState->GetPlayerName());

	// 3. Domain Service Call
	DomainService->SaveInventory(PlayerState, InventoryData);
}

