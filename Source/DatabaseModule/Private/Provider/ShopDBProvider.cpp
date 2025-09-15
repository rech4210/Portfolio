
#include "Provider/ShopDBProvider.h"
#include "DatabaseManager.h"

UE::Tasks::TTask<FShopRepositoryResult> ShopDBProvider::LoadShopByID(int32 ShopID) {
	return DBManager->LoadShopByID(ShopID);
}
UE::Tasks::TTask<bool> ShopDBProvider::SaveShop(const FShopDomain& ShopData) {
	return DBManager->SaveShop(ShopData);
}
UE::Tasks::TTask<TArray<FShopRepositoryResult>> ShopDBProvider::LoadShopsByIDs(const TArray<int32>& ShopIDs) {
	return DBManager->LoadShopsByIDs(ShopIDs);
}
UE::Tasks::TTask<TArray<FShopRepositoryResult>> ShopDBProvider::LoadShopsForArea(int32 AreaID) {
	return DBManager->LoadShopsForArea(AreaID);
}
UE::Tasks::TTask<bool> ShopDBProvider::DeleteShop(int32 ShopID) {
	return DBManager->DeleteShop(ShopID);
}
UE::Tasks::TTask<bool> ShopDBProvider::CheckShopExists(int32 ShopID) {
	return DBManager->CheckShopExists(ShopID);
}
UE::Tasks::TTask<bool> ShopDBProvider::AddItemToShop(int32 ShopID, const FShopItemDTO& ItemData) {
	return DBManager->AddItemToShop(ShopID, ItemData);
}
UE::Tasks::TTask<bool> ShopDBProvider::RemoveItemFromShop(int32 ShopID, int32 ItemID) {
	return DBManager->RemoveItemFromShop(ShopID, ItemID);
}
UE::Tasks::TTask<bool> ShopDBProvider::UpdateShopItemStock(int32 ShopID, int32 ItemID, int32 NewStock) {
	return DBManager->UpdateShopItemStock(ShopID, ItemID, NewStock);
}
UE::Tasks::TTask<bool> ShopDBProvider::UpdateShopItemPrice(int32 ShopID, int32 ItemID, float NewPrice) {
	return DBManager->UpdateShopItemPrice(ShopID, ItemID, NewPrice);
}

