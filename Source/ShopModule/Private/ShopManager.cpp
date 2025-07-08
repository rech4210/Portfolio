#include "ShopSeller.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "GameFramework/Character.h"

AShopSeller::AShopSeller()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ShopComponent = CreateDefaultSubobject<UShopComponent>(TEXT("ShopComponent"));
	ShopComponent->SetIsReplicated(true);
}
