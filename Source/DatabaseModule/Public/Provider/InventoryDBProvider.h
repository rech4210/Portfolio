#pragma once
#include "Interface/InventoryRepositoryInterface.h"
#include "Interface/Provider/IInventoryDBProvider.h"
#include "DatabaseManager.h"
class InventoryDBProvider :public IInventoryDBProvider {
public:
	InventoryDBProvider(UDatabaseManager * InDBManager): DBManager(InDBManager) {}
public:
	virtual UE::Tasks::TTask<TArray<FInventoryItemDTO>> LoadInventoryForPlayer(const FString& UserId) override;
	virtual UE::Tasks::TTask<bool> SaveInventoryForPlayer(const FString& UserId, const TArray<FInventoryItemDTO>& Items) override;
	virtual UE::Tasks::TTask<bool> AddInventoryItem(const FString& UserId, const FInventoryItemDTO& Item) override;
	virtual UE::Tasks::TTask<bool> RemoveInventoryItem(const FString& UserId, const FName& ItemID, int32 Quantity) override;
private:
	TWeakObjectPtr<UDatabaseManager> DBManager;
};
