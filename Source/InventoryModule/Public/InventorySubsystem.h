// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventoryRepositoryInterface.h"
#include "InventorySubsystem.generated.h"

class UInventoryComponent;
class UInventoryRepository;

/**
 * 
 */
UCLASS()
class INVENTORYMODULE_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<IInventoryRepositoryInterface> GetInventoryRepository() const;

	/** Entry point for loading player inventory data. Can be called from PlayerState's BeginPlay. */
	void RequestLoadInventory(APlayerState* PlayerState);

	/** Called by the InventoryComponent on clients when inventory data is replicated. */
	void Client_OnInventoryUpdated(UInventoryComponent* InventoryComponent);

private:
	UPROPERTY()
	UInventoryRepository* InventoryRepository;
};