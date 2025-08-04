
#pragma once

#include "CoreMinimal.h"
#include "InventoryRepositoryInterface.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "InventoryRepository.generated.h"

class UInventoryComponent;
class UDatabaseManager;
struct FInventoryItemDTO;

UCLASS()
class INVENTORYMODULE_API UInventoryRepository : public UObject, public IInventoryRepositoryInterface 
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;

	// ========================================================================
	// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES
	// ========================================================================
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> LoadInventoryByPlayerId(const FGuid& PlayerId) override;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> SaveInventoryData(const FInventoryDomain& InventoryData) override;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> AddItemByPlayerId(
		const FGuid& PlayerId, const FInventoryItemDTO& Item) override;
	virtual UE::Tasks::TTask<FInventoryRepositoryResult> RemoveItemByPlayerId(
		const FGuid& PlayerId, const FName& ItemID, int32 Quantity) override;
private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};