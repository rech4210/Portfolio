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
