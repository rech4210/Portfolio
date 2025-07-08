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
	virtual void Initialize() override;
	virtual void RequestLoadInventoryForPlayer(APlayerState* PlayerState) override;
	virtual void RequestSaveInventoryForPlayer(APlayerState* PlayerState) override;
	virtual UE::Tasks::TTask<bool> LoadInventoryForPlayer(APlayerState* PlayerState) override;
	virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(APlayerState* PlayerState) override;
	virtual UE::Tasks::TTask<bool> AddItemToPlayer(APlayerState* PlayerState, const FInventoryItemDTO& Item) override;
	virtual UE::Tasks::TTask<bool> RemoveItemFromPlayer(APlayerState* PlayerState, const FName& ItemID, int32 Quantity) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};