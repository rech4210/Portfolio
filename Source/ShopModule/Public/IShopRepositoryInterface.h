// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "Tasks/Task.h"
#include "Interface/BaseRepositoryInterface.h"
#include "IShopRepositoryInterface.generated.h"

class UShopComponent;
struct FShopDomain;
struct FShopRepositoryResult;

// This class does not need to be modified.
UINTERFACE()
class UShopRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()

public:
};

/**
 * Repository interface for shop data persistence operations.
 * Provides both legacy (component-based) and DDD (domain-based) methods.
 */
class SHOPMODULE_API IShopRepositoryInterface : public IBaseRepositoryInterface{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// ============================================================================
	// DDD-based methods (pure domain operations)
	// ============================================================================
	
	/**
	 * Load shop data by shop ID (pure domain operation)
	 * @param ShopID The shop identifier
	 * @return Task that completes with shop repository result
	 */
	virtual UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID) = 0;

	/**
	 * Save shop domain data (pure domain operation)
	 * @param ShopData The shop domain object to save
	 * @return Task that completes when save operation finishes
	 */
	virtual UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData) = 0;

	/**
	 * Load multiple shops by IDs (pure domain operation)
	 * @param ShopIDs Array of shop identifiers
	 * @return Task that completes with array of shop repository results
	 */
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs) = 0;

	/**
	 * Load all shops for a specific area/region (pure domain operation)
	 * @param AreaID The area identifier
	 * @return Task that completes with array of shop repository results
	 */
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID) = 0;

	/**
	 * Delete a shop by ID (pure domain operation)
	 * @param ShopID The shop identifier
	 * @return Task that completes when delete operation finishes
	 */
	virtual UE::Tasks::TTask<bool> DeleteShop(int32 ShopID) = 0;

	/**
	 * Check if a shop exists (pure domain operation)
	 * @param ShopID The shop identifier
	 * @return Task that completes with existence check result
	 */
	virtual UE::Tasks::TTask<bool> ShopExists(int32 ShopID) = 0;

	// ============================================================================
	// Legacy methods (component-based operations) - for backward compatibility
	// ============================================================================
	
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
