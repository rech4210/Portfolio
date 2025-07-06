// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShopComponent.generated.h"

class UShopItemData;

USTRUCT(BlueprintType)
struct FShopItemState
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	int32 Stock = 0;

	UPROPERTY()
	float Price = 0.0f;

	UPROPERTY()
	bool bIsAvailable = true;
};

/**
 * 상점 아이템들을 관리하는 컴포넌트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopStateChanged, const TArray<FShopItemState>&, UpdatedItems);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOPMODULE_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShopComponent();

	// 상점 상태가 변경될 때 호출되는 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopStateChanged OnShopStateChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ShopItems, VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	TArray<FShopItemState> ShopItems;

public:
	// 상점 아이템 관리
	bool AddShopItem(const FShopItemState& ItemState);
	bool RemoveShopItem(int32 ItemID);
	bool UpdateItemStock(int32 ItemID, int32 NewStock);
	bool UpdateItemPrice(int32 ItemID, float NewPrice);
	
	const TArray<FShopItemState>& GetAllShopItems() const { return ShopItems; }
	FShopItemState* GetShopItem(int32 ItemID);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_ShopItems();
};
