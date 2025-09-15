// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Tasks/Task.h"

struct FInventoryItemDTO; // forward
#include "IInventoryDBProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInventoryDBProvider : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class GAMESHAREDMODULE_API IInventoryDBProvider {
	GENERATED_BODY()
public:
	// Inventory CRUD
	virtual UE::Tasks::TTask<TArray<FInventoryItemDTO>> LoadInventoryForPlayer(const FString& UserId) = 0;
	virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(const FString& UserId, const TArray<FInventoryItemDTO>& Items) = 0;
	virtual UE::Tasks::TTask<bool> AddInventoryItem(const FString& UserId, const FInventoryItemDTO& Item) = 0;
	virtual UE::Tasks::TTask<bool> RemoveInventoryItem(const FString& UserId, const FName& ItemID, int32 Quantity) = 0;
};
