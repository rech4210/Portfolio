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

UInventoryDomainService* UInventorySubsystem::CreateDomainService()
{
	UInventoryDomainService* DomainService = NewObject<UInventoryDomainService>(this);
	DomainService->Initialize(InventoryRepositoryInterface);
	
	UE_LOG(LogTemp, Log, TEXT("InventorySubsystem: Created new InventoryDomainService"));
	return DomainService;
}

void UInventorySubsystem::RequestLoadInventory(APlayerState* PlayerState)
{
	UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem::RequestLoadInventory is deprecated. Use InventoryDomainService instead."));
	
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		// Clients don't load data from the database. They wait for replication.
		return;
	}

	if (InventoryRepositoryInterface.GetInterface() && PlayerState)
	{
		// Legacy support - just call repository directly
		InventoryRepositoryInterface->RequestLoadInventoryForPlayer(PlayerState);
	}
}

void UInventorySubsystem::Client_OnInventoryUpdated(UInventoryComponent* InventoryComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem::Client_OnInventoryUpdated is deprecated. Use InventoryDomainService instead."));
	
	// Legacy support - minimal implementation
	if (InventoryRepositoryInterface.GetInterface())
	{
		// The repository can perform any client-side logic, like updating local caches or data assets.
	}
}