// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryRepositoryInterface.h"
#include "UObject/Object.h"
#include "InventoryRepository.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYMODULE_API UInventoryRepository : public UObject, public IInventoryRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void LoadInventoryForPlayer(const TObjectPtr<APlayerState>& Object) override;
	virtual void SaveInventoryForPlayer(const TObjectPtr<APlayerState>& Object) override;
};
