// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Tasks/Task.h"
#include "GameSharedModule/Public/DTO/ShopDTOs.h"
#include "IShopDBProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UShopDBProvider : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class GAMESHAREDMODULE_API IShopDBProvider {
	GENERATED_BODY()
public:
	virtual UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID) = 0;
	virtual UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData) = 0;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs) = 0;
	virtual UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID) = 0;
	virtual UE::Tasks::TTask<bool> DeleteShop(int32 ShopID) = 0;
	virtual UE::Tasks::TTask<bool> CheckShopExists(int32 ShopID) = 0;
	virtual UE::Tasks::TTask<bool> AddItemToShop(int32 ShopID, const FShopItemDTO& ItemData) = 0;
	virtual UE::Tasks::TTask<bool> RemoveItemFromShop(int32 ShopID, int32 ItemID) = 0;
	virtual UE::Tasks::TTask<bool> UpdateShopItemStock(int32 ShopID, int32 ItemID, int32 NewStock) = 0;
	virtual UE::Tasks::TTask<bool> UpdateShopItemPrice(int32 ShopID, int32 ItemID, float NewPrice) = 0;
};
