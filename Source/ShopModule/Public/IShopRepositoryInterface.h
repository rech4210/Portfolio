
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "Tasks/Task.h"
#include "Interface/BaseRepositoryInterface.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "IShopRepositoryInterface.generated.h"

class UShopComponent;
UINTERFACE()
class UShopRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()
};

class SHOPMODULE_API IShopRepositoryInterface : public IBaseRepositoryInterface{
	GENERATED_BODY()
public:
	
	// ============================================================================
	// DDD-based methods (pure domain operations)
	// ============================================================================

	virtual UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID) = 0;
	virtual UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData) = 0;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs) = 0;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID) = 0;
	virtual UE::Tasks::TTask<bool> DeleteShop(int32 ShopID) = 0;
	virtual UE::Tasks::TTask<bool> ShopExists(int32 ShopID) = 0;
};
