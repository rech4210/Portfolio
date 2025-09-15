
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Tasks/Task.h"
#include "Interface/BaseRepositoryInterface.h"
#include "GameSharedModule/Public/DTO/ShopDTOs.h"
#include "IShopRepositoryInterface.generated.h"

// Shop DTOs now provided via ShopDTOs.h

class UShopComponent;
UINTERFACE()
class UShopRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IShopRepositoryInterface : public IBaseRepositoryInterface{
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
