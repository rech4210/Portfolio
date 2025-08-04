
#pragma once

#include "CoreMinimal.h"
#include "IShopRepositoryInterface.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "ShopRepository.generated.h"

class UShopComponent;
class UDatabaseManager;

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
	
	virtual UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID) override;
	virtual UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData) override;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs) override;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID) override;
	virtual UE::Tasks::TTask<bool> DeleteShop(int32 ShopID) override;
	virtual UE::Tasks::TTask<bool> ShopExists(int32 ShopID) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};
