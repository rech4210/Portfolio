
#pragma once

#include "CoreMinimal.h"
#include "IShopRepositoryInterface.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "DatabaseModule/Public/DatabaseManager.h" // Import DTO types from DatabaseModule
#include "ShopRepository.generated.h"

class UShopComponent;
class UDatabaseManager;

/**
 * Repository implementation for shop data persistence.
 * Provides both legacy (component-based) and DDD (domain-based) operations.
 */
UCLASS()
class SHOPMODULE_API UShopRepository : public UObject, public IShopRepositoryInterface {
	GENERATED_BODY()

public:
	// ============================================================================
	// Base repository interface
	// ============================================================================
	virtual void Initialize() override;

	// ============================================================================
	// DDD-based methods (pure domain operations)
	// ============================================================================
	
	/**
	 * Load shop data by shop ID (pure domain operation)
	 * @param ShopID The shop identifier
	 * @return Task that completes with shop repository result
	 */
	virtual UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID) override;

	/**
	 * Save shop domain data (pure domain operation)
	 * @param ShopData The shop domain object to save
	 * @return Task that completes when save operation finishes
	 */
	virtual UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData) override;

	/**
	 * Load multiple shops by IDs (pure domain operation)
	 * @param ShopIDs Array of shop identifiers
	 * @return Task that completes with array of shop repository results
	 */
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs) override;

	/**
	 * Load all shops for a specific area/region (pure domain operation)
	 * @param AreaID The area identifier
	 * @return Task that completes with array of shop repository results
	 */
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID) override;

	/**
	 * Delete a shop by ID (pure domain operation)
	 * @param ShopID The shop identifier
	 * @return Task that completes when delete operation finishes
	 */
	virtual UE::Tasks::TTask<bool> DeleteShop(int32 ShopID) override;

	/**
	 * Check if a shop exists (pure domain operation)
	 * @param ShopID The shop identifier
	 * @return Task that completes with existence check result
	 */
	virtual UE::Tasks::TTask<bool> ShopExists(int32 ShopID) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;

	/**
	 * Create a mock shop domain for testing (temporary implementation)
	 * @param ShopID The shop identifier
	 * @return Mock shop domain object
	 */
	// FShopDomain CreateMockShopData(int32 ShopID);
};
