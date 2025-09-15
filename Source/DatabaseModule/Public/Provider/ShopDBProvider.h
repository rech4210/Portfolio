#pragma once
#include "Interface/Provider/IShopDBProvider.h"
#include "DatabaseManager.h"
class ShopDBProvider : public IShopDBProvider{
public:
	ShopDBProvider(UDatabaseManager * InDBManager): DBManager(InDBManager) {}
	virtual UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID) override;
	virtual UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData) override;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs) override;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID) override;
	virtual UE::Tasks::TTask<bool> DeleteShop(int32 ShopID) override;
	virtual UE::Tasks::TTask<bool> CheckShopExists(int32 ShopID) override;
	virtual UE::Tasks::TTask<bool> AddItemToShop(int32 ShopID, const FShopItemDTO& ItemData) override;
	virtual UE::Tasks::TTask<bool> RemoveItemFromShop(int32 ShopID, int32 ItemID) override;
	virtual UE::Tasks::TTask<bool> UpdateShopItemStock(int32 ShopID, int32 ItemID, int32 NewStock) override;
	virtual UE::Tasks::TTask<bool> UpdateShopItemPrice(int32 ShopID, int32 ItemID, float NewPrice) override;
	/*DB APIs from interface*/
private:
	TWeakObjectPtr<UDatabaseManager> DBManager;
};
