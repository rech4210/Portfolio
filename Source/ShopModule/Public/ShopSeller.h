// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ShopComponent.h"
#include "GameFramework/Actor.h"
#include "ShopSeller.generated.h"

class UInventoryComponent;
class UItemDataAsset;
class ACharacter;

/**
 * An actor that represents a shop in the world.
 * This class is responsible for managing the shop's own inventory.
 * The actual transaction logic is handled by the UTradingService.
 */
UCLASS()
class SHOPMODULE_API AShopSeller : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UShopComponent> ShopComponent;

public:	
	AShopSeller();
	UFUNCTION(BlueprintPure, Category = "Shop")
	UShopComponent* GetShopComponent() const { return ShopComponent; }
}; 