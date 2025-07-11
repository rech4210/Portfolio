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
	SetupDomainEventHandlers();
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

UShopDomainService* UShopSubsystem::GetShopDomainService() const
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

// ============================================================================
// Legacy Entry Points (for backward compatibility)
// ============================================================================

void UShopSubsystem::RequestLoadShopData(APlayerState* PlayerState)
{
	// Legacy method - for backward compatibility
	// 클라이언트는 DB에서 데이터를 로드하지 않고 복제를 기다립니다.
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	if (ShopRepository && PlayerState)
	{
		// 서버에서만 실행: 비동기적으로 DB 또는 외부 저장소에서 상점 데이터 로드
		if (auto* ShopComponent = PlayerState->FindComponentByClass<UShopComponent>())
		{
			// 서버 권한이 있을 때만 ShopComponent의 복제된 프로퍼티를 수정
			if (PlayerState->HasAuthority())
			{
				// ShopRepository를 통해 상점 데이터 로드
				ShopRepository->LoadShopData(PlayerState->GetPlayerId(), *ShopComponent);
			}
		}
	}
}

void UShopSubsystem::Client_OnShopStateUpdated(UShopComponent* ShopComponent)
{
	if (ShopRepository && ShopComponent)
	{
		// 클라이언트 측에서 복제된 데이터를 받았을 때의 로직
		// 로컬 데이터 에셋이나 캐시를 로드·적용하고, 필요한 로직을 수행
		
		UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Client received shop state update for %d items"), 
			ShopComponent->GetAllShopItems().Num());
	}
}

// ============================================================================
// Private Methods
// ============================================================================

void UShopSubsystem::SetupDomainEventHandlers()
{
	if (!ShopDomainService)
	{
		return;
	}

	// Subscribe to domain events
	ShopDomainService->OnItemPurchased.AddUObject(this, &UShopSubsystem::OnItemPurchased);
	ShopDomainService->OnShopLoaded.AddUObject(this, &UShopSubsystem::OnShopLoaded);
	ShopDomainService->OnShopSaved.AddUObject(this, &UShopSubsystem::OnShopSaved);
}

// ============================================================================
// Domain Event Handlers - UI Updates & Client Events Only
// ============================================================================

void UShopSubsystem::OnItemPurchased(APlayerState* PlayerState, int32 ItemID, int32 Quantity)
{
	if (!PlayerState)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Domain event - Item purchased by %s: Item %d, Quantity %d"), 
		*PlayerState->GetPlayerName(), ItemID, Quantity);

	// App Layer responsibilities for Aggregate changes:
	// 1. UI Updates via Component synchronization
	if (auto* ShopComponent = PlayerState->FindComponentByClass<UShopComponent>())
	{
		// Trigger UI refresh for purchase completion
		ShopComponent->OnItemPurchasedEvent.Broadcast(ItemID, Quantity);
	}

	// 2. Cross-system notifications (Analytics, Achievements, etc.)
	// Analytics system for purchase tracking
	// Achievement system for purchase milestones
	// Economy system for currency tracking
	// Inventory system integration (should be handled separately)
	
	// 3. Client-side event broadcasting
	// Broadcast to UI systems, sound systems, VFX systems
}

void UShopSubsystem::OnShopLoaded(APlayerState* PlayerState, const FShopDomain& ShopData)
{
	if (!PlayerState)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Domain event - Shop loaded for %s: Shop %d (%s) with %d items"), 
		*PlayerState->GetPlayerName(), ShopData.ShopID, *ShopData.ShopName, ShopData.ShopItems.Num());

	// App Layer responsibilities for Aggregate changes:
	// 1. UI System updates
	if (auto* ShopComponent = PlayerState->FindComponentByClass<UShopComponent>())
	{
		// Synchronize component with domain state
		ShopComponent->SyncWithDomain(ShopData);
		
		// Broadcast shop data loaded event for UI systems
		ShopComponent->OnShopDataLoadedEvent.Broadcast(ShopData);
	}

	// 2. Client-side state synchronization
	// Update local caches, UI displays, etc.
	
	// 3. Cross-cutting concerns
	// Performance metrics, loading time analytics
}

void UShopSubsystem::OnShopSaved(const FShopDomain& ShopData)
{
	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Domain event - Shop saved: Shop %d (%s) with %d items"), 
		ShopData.ShopID, *ShopData.ShopName, ShopData.ShopItems.Num());

	// App Layer responsibilities for persistence events:
	// 1. Backup and versioning systems
	// Create save points, version tracking
	
	// 2. Analytics for shop state changes
	// Track shop modification patterns, admin actions
	
	// 3. Audit logging
	// Log who made what changes when
	
	// 4. Replication triggers (if needed)
	// Notify clients of shop data changes
}
