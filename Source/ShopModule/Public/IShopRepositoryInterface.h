// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "Interface/BaseRepositoryInterface.h"
#include "IShopRepositoryInterface.generated.h"

class UShopComponent;

// This class does not need to be modified.
UINTERFACE()
class UShopRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()

public:
};

/**
 * 
 */
class SHOPMODULE_API IShopRepositoryInterface : public IBaseRepositoryInterface{
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

	/**
	 * Loads shop data for a specific player/shop component.
	 * @param PlayerInformation Player ID or identifier
	 * @param ShopComponentToPopulate The shop component to populate with data
	 * @return True if loading was successful
	 */
	virtual bool LoadShopData(int32 PlayerInformation, UShopComponent& ShopComponentToPopulate) = 0;

	/**
	 * Saves shop data from a shop component.
	 * @param PlayerInformation Player ID or identifier
	 * @param ShopComponentToSave The shop component containing data to save
	 * @return True if saving was successful
	 */
	virtual bool SaveShopData(int32 PlayerInformation, const UShopComponent* ShopComponentToSave) = 0;
};
