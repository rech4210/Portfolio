#include "ShopManager.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "GameFramework/Character.h"

AShopManager::AShopManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// The shop manager should be replicated if we want to see its state on clients.
	bReplicates = true;
}

bool AShopManager::CanBuyItem(ACharacter* Buyer, const UItemDataAsset* ItemToBuy) const
{
	if (!Buyer || !ItemToBuy)
	{
		return false;
	}

	// Check if the item is in the list of items for sale.
	return ItemsForSale.Contains(ItemToBuy);
}

bool AShopManager::CanSellItem(ACharacter* Seller, const UItemDataAsset* ItemToSell) const
{
	if (!Seller || !ItemToSell)
	{
		return false;
	}

	// For now, the shop can buy any item.
	// Future logic could check for item type, condition, etc.
	return true;
} 