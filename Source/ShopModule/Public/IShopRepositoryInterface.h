// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "IShopRepositoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UShopRepositoryInterface : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class SHOPMODULE_API IShopRepositoryInterface {
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
 * Asynchronously loads all shop data for a given world/level.
 * This should find all AShopManager actors and populate their inventories.
 * @param WorldContextObject A UObject to get the World from.
 * @return A future that completes when all shops have been loaded.
 */
	virtual TFuture<void> LoadAllShops(const UObject* WorldContextObject) = 0;
};
