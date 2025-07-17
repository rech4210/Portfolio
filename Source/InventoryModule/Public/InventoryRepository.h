// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryRepositoryInterface.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "InventoryRepository.generated.h"

class UInventoryComponent;
class UDatabaseManager;
struct FInventoryItemDTO;

/**
 * Repository for managing inventory persistence with async database operations
 */
UCLASS()
class INVENTORYMODULE_API UInventoryRepository : public UObject, public IInventoryRepositoryInterface 
{
	GENERATED_BODY()

public:
	// IInventoryRepositoryInterface implementation
	virtual void Initialize() override;

	// ========================================================================
	// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES (RECOMMENDED)
	// ========================================================================
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> LoadInventoryByPlayerId(const FGuid& PlayerId) override;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> SaveInventoryData(const FInventoryDomain& InventoryData) override;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> AddItemByPlayerId(
		const FGuid& PlayerId, const FInventoryItemDTO& Item) override;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> RemoveItemByPlayerId(
		const FGuid& PlayerId, const FName& ItemID, int32 Quantity) override;

	// Legacy support methods
	// virtual void RequestLoadInventoryForPlayer(APlayerState* PlayerState) override;
	// virtual void RequestSaveInventoryForPlayer(APlayerState* PlayerState) override;
	// virtual UE::Tasks::TTask<bool> LoadInventoryForPlayer(APlayerState* PlayerState);
	// virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(APlayerState* PlayerState);
	// virtual UE::Tasks::TTask<bool> AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item);
	// virtual UE::Tasks::TTask<bool> RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity);

	// ========================================================================
	// DEPRECATED METHODS - ENGINE OBJECT DEPENDENCIES (LEGACY SUPPORT)
	// ========================================================================
	// virtual void RequestLoadInventoryForPlayer(APlayerState* PlayerState) override;
	// virtual void RequestSaveInventoryForPlayer(APlayerState* PlayerState) override;
	// virtual UE::Tasks::TTask<bool> LoadInventoryForPlayer(APlayerState* PlayerState) override;
	// virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(APlayerState* PlayerState) override;
	// virtual UE::Tasks::TTask<bool> AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item) override;
	// virtual UE::Tasks::TTask<bool> RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};