// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "InventoryRepository.h"
#include "InventoryComponent.h"
#include "InventoryDomainService.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"

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

void UInventorySubsystem::RequestAddItemToInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryItemDTO& Item)
{
	// 1. Network & Authority Validation (App Layer responsibility)
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerIdentity for item addition"));
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
		*PlayerIdentity->GetPlayerGuid().ToString(), *Item.ItemID.ToString(), Item.Quantity);

	// 3. Domain Service Call (Delegate business logic)
	DomainService->AddItemToInventory(PlayerIdentity, Item);
}

void UInventorySubsystem::RequestRemoveItemFromInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FName& ItemID, int32 Quantity)
{
	// 1. Network & Authority Validation
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerIdentity for item removal"));
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
		*PlayerIdentity->GetPlayerGuid().ToString(), *ItemID.ToString(), Quantity);

	// 3. Domain Service Call
	DomainService->RemoveItemFromInventory(PlayerIdentity, ItemID, Quantity);
}

void UInventorySubsystem::RequestLoadPlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity)
{
	// 1. Network & Authority Validation
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerIdentity for inventory loading"));
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
		*PlayerIdentity->GetPlayerGuid().ToString());

	// 3. Domain Service Call
	DomainService->LoadInventory(PlayerIdentity);
}

void UInventorySubsystem::RequestSavePlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryDomain& InventoryData)
{
	// 1. Authority Validation
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem: Invalid PlayerIdentity for inventory saving"));
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
		*PlayerIdentity->GetPlayerGuid().ToString());

	// 3. Domain Service Call
	DomainService->SaveInventory(PlayerIdentity, InventoryData);
}

