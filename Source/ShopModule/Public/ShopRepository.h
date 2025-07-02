// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IShopRepositoryInterface.h"
#include "UObject/Object.h"
#include "ShopRepository.generated.h"

/**
 * 
 */
UCLASS()
class SHOPMODULE_API UShopRepository : public UObject, public IShopRepositoryInterface {
	GENERATED_BODY()

public:
	virtual TFuture<void> LoadAllShops(const UObject* WorldContextObject) override;
};
