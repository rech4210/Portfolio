
#include "Provider/InventoryDBProvider.h"
#include "DatabaseManager.h"

UE::Tasks::TTask<TArray<FInventoryItemDTO>> InventoryDBProvider::LoadInventoryForPlayer(const FString& UserId) {
	return DBManager->LoadInventoryForPlayer(UserId);
}

UE::Tasks::TTask<bool> InventoryDBProvider::SaveInventoryForPlayer(const FString& UserId, const TArray<FInventoryItemDTO>& Items) {
	return DBManager->SaveInventoryForPlayer(UserId, Items);
}

UE::Tasks::TTask<bool> InventoryDBProvider::AddInventoryItem(const FString& UserId, const FInventoryItemDTO& Item) {
	return DBManager->AddInventoryItem(UserId, Item);
}

UE::Tasks::TTask<bool> InventoryDBProvider::RemoveInventoryItem(const FString& UserId, const FName& ItemID, int32 Quantity) {
	return DBManager->RemoveInventoryItem(UserId, ItemID, Quantity);
}
