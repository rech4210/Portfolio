// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryRepositoryInterface.generated.h"

class APlayerState;

// This class does not need to be modified.
UINTERFACE()
class UInventoryRepositoryInterface : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYMODULE_API IInventoryRepositoryInterface {
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void LoadInventoryForPlayer(APlayerState* PlayerState) = 0;
	virtual void SaveInventoryForPlayer(APlayerState* PlayerState) = 0;
};