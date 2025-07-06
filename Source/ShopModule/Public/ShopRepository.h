// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IShopRepositoryInterface.h"
#include "UObject/Object.h"
#include "ShopRepository.generated.h"

class UShopComponent;

/**
 * 
 */
UCLASS()
class SHOPMODULE_API UShopRepository : public UObject, public IShopRepositoryInterface {
	GENERATED_BODY()

public:
	virtual TFuture<void> LoadAllShops(const UObject* WorldContextObject) override;
	virtual bool LoadShopData(int32 PlayerInformation, UShopComponent& ShopComponentToPopulate) override;
	virtual bool SaveShopData(int32 PlayerInformation, const UShopComponent* ShopComponentToSave) override;
};
