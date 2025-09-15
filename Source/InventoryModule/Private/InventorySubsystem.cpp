
#include "InventorySubsystem.h"
#include "InventoryRepository.h"
#include "InventoryComponent.h"
#include "InventoryDomainService.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"
#include "DatabaseModule/Public/Provider/DBProviderInfra.h"

void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UDBProviderInfra::StaticClass());
	Super::Initialize(Collection);
	
	DefaultInventoryRepository = NewObject<UInventoryRepository>(this, TEXT("DefaultInventoryRepository"));
	if (UDBProviderInfra* Infra = GetGameInstance()->GetSubsystem<UDBProviderInfra>())
	{
		DefaultInventoryRepository->Initialize(Infra);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("InventorySubsystem: UDBProviderInfra not available"));
	}
	
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

	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting add item transaction - Player: %s, Item: %s, Quantity: %d"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *Item.ItemID.ToString(), Item.Quantity);

	DomainService->AddItemToInventory(PlayerIdentity, Item);
}

void UInventorySubsystem::RequestRemoveItemFromInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FName& ItemID, int32 Quantity)
{
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

	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting remove item transaction - Player: %s, Item: %s, Quantity: %d"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *ItemID.ToString(), Quantity);

	DomainService->RemoveItemFromInventory(PlayerIdentity, ItemID, Quantity);
}

void UInventorySubsystem::RequestLoadPlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity)
{
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

	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting inventory load transaction - Player: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString());

	DomainService->LoadInventory(PlayerIdentity);
}

void UInventorySubsystem::RequestSavePlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryDomain& InventoryData)
{
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

	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Starting inventory save transaction - Player: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString());

	DomainService->SaveInventory(PlayerIdentity, InventoryData);
}

