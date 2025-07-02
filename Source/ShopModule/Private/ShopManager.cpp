#include "ShopManager.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "GameFramework/Character.h"

AShopManager::AShopManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ShopInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("ShopInventory"));
	ShopInventory->SetIsReplicated(true);
}

bool AShopManager::HasItemForSale(const UItemDataAsset* ItemToBuy, int32 Quantity) const
{
	if (!ItemToBuy || !ShopInventory)
	{
		return false;
	}

	return ShopInventory->HasItem(ItemToBuy, Quantity);
}