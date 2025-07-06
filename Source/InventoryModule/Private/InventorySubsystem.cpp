// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"
#include "InventoryRepository.h"
#include "GameFramework/PlayerState.h"

void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InventoryRepository = NewObject<UInventoryRepository>(this, TEXT("InventoryRepository"));
}

void UInventorySubsystem::Deinitialize()
{
	InventoryRepository = nullptr;
	Super::Deinitialize();
}

TScriptInterface<IInventoryRepositoryInterface> UInventorySubsystem::GetInventoryRepository() const
{
	return InventoryRepository;
}

void UInventorySubsystem::RequestLoadInventory(APlayerState* PlayerState)
{
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		// Clients don't load data from the database. They wait for replication.
		return;
	}

	if (InventoryRepository && PlayerState)
	{
		// This is where you would trigger an async load from a database or other persistent storage.
		// For now, we'll simulate a load and directly apply it.
		InventoryRepository->LoadInventoryForPlayer(PlayerState);
	}
}

void UInventorySubsystem::Client_OnInventoryUpdated(UInventoryComponent* InventoryComponent)
{
	if (InventoryRepository)
	{
		// The repository can now perform any client-side logic, like updating local caches or data assets.
		// For example, it might register the items with some other system.
	}
}