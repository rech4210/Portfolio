// Fill out your copyright notice in the Description page of Project Settings.

#include "ShopSubsystem.h"
#include "DatabaseManager.h"
#include "ShopRepository.h"
#include "ShopDomainService.h"
#include "ShopDomain.h"
#include "Components/ShopComponent.h"
#include "GameFramework/PlayerState.h"

void UShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	
	// Initialize repository (DI/DIP pattern)
	ShopRepository = NewObject<UShopRepository>(this, TEXT("ShopRepository"));
	ShopRepository->Initialize();
	
	// Initialize domain service with repository dependency injection
	ShopDomainService = NewObject<UShopDomainService>(this, TEXT("ShopDomainService"));
	ShopDomainService->Initialize(ShopRepository);
	
	// Setup domain event handlers for cross-cutting concerns
	// SetupDomainEventHandlers();
}

void UShopSubsystem::Deinitialize()
{
	if (ShopDomainService)
	{
		// Clean up domain event subscriptions
		ShopDomainService->OnItemPurchased.RemoveAll(this);
		ShopDomainService->OnShopLoaded.RemoveAll(this);
		ShopDomainService->OnShopSaved.RemoveAll(this);
	}
	
	ShopRepository = nullptr;
	ShopDomainService = nullptr;
	Super::Deinitialize();
}

TScriptInterface<IShopRepositoryInterface> UShopSubsystem::GetShopRepository() const
{
	return ShopRepository;
}

UShopDomainService* UShopSubsystem::GetDomainService() const
{
	return ShopDomainService;
}

// ============================================================================
// Use Case Orchestration - App Layer Responsibilities Only
// ============================================================================

void UShopSubsystem::RequestPurchaseItem(APlayerState* PlayerState, int32 ShopID, int32 ItemID, int32 Quantity, float PlayerCurrency)
{
	// 1. Network & Authority Validation (App Layer responsibility)
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Invalid PlayerState for purchase"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Purchase requests should only be made from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging (App Layer responsibility)
	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting purchase transaction - Player: %s, Shop: %d, Item: %d, Quantity: %d"), 
		*PlayerState->GetPlayerName(), ShopID, ItemID, Quantity);

	// 3. Domain Service Call (Delegate business logic)
	ShopDomainService->PurchaseItem(PlayerState, ShopID, ItemID, Quantity, PlayerCurrency);
}

void UShopSubsystem::RequestLoadShopData(APlayerState* PlayerState, int32 ShopID)
{
	// 1. Network & Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Invalid PlayerState for shop loading"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Client should wait for replicated data, not load from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting shop load transaction - Player: %s, Shop: %d"), 
		*PlayerState->GetPlayerName(), ShopID);

	// 3. Domain Service Call
	ShopDomainService->LoadShop(PlayerState, ShopID);
}

void UShopSubsystem::RequestAddItemToShop(int32 ShopID, int32 ItemID, int32 Stock, float Price)
{
	// 1. Authority Validation
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Item addition should only be requested from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting add item transaction - Shop: %d, Item: %d, Stock: %d, Price: %.2f"), 
		ShopID, ItemID, Stock, Price);

	// 3. Create DTO and delegate to domain service
	FShopItemDTO NewItem(ItemID, Stock, Price, ShopID);
	ShopDomainService->AddItemToShop(ShopID, NewItem);
}

void UShopSubsystem::RequestRemoveItemFromShop(int32 ShopID, int32 ItemID)
{
	// 1. Authority Validation
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Item removal should only be requested from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting remove item transaction - Shop: %d, Item: %d"), ShopID, ItemID);

	// 3. Domain Service Call
	ShopDomainService->RemoveItemFromShop(ShopID, ItemID);
}

void UShopSubsystem::RequestRestockShop(int32 ShopID)
{
	// 1. Authority Validation
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Restock should only be requested from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting restock transaction - Shop: %d"), ShopID);

	// 3. Domain Service Call
	ShopDomainService->RestockShop(ShopID);
}
