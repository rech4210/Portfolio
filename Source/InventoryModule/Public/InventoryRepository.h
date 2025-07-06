// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryRepositoryInterface.h"
#include "UObject/Object.h"
#include "InventoryRepository.generated.h"

class UInventoryComponent;

/**
 * 
 */
UCLASS()
class INVENTORYMODULE_API UInventoryRepository : public UObject, public IInventoryRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void LoadInventoryForPlayer(APlayerState* PlayerState) override;
	virtual void SaveInventoryForPlayer(APlayerState* PlayerState) override;

	/** Applies loaded data to the server-side component. */
	void Server_ApplyInventoryData(UInventoryComponent* InventoryComponent, const TArray<struct FInventoryItemData>& LoadedItems);

	/** Handles client-side updates after replication. */
	void Client_OnInventoryUpdated(UInventoryComponent* InventoryComponent);
};