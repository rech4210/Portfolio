
#pragma once

#include "CoreMinimal.h"
#include "GameSharedModule/Public/Interface/BaseRepositoryInterface.h"
#include "UObject/Interface.h"
#include "InventoryDomain.h"
#include "InventoryRepositoryInterface.generated.h"

class APlayerState;

UINTERFACE()
class UInventoryRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()
};

class INVENTORYMODULE_API IInventoryRepositoryInterface : public IBaseRepositoryInterface {
	GENERATED_BODY()
public:
	
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> LoadInventoryByPlayerId(const FGuid& PlayerId) = 0;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> SaveInventoryData(const FInventoryDomain& InventoryData) = 0;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> AddItemByPlayerId(const FGuid& PlayerId, const FInventoryItemDTO& Item) = 0;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> RemoveItemByPlayerId(const FGuid& PlayerId, const FName& ItemID, int32 Quantity) = 0;
};