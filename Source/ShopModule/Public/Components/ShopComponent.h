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
 *  실질적으로 ItemDataAsset이 아닌, 표면적인 데이터를 다룬다. ItemDataAsset은 부가 기능이 많기 때문.
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
	bool UpdateShopItem(int32 ItemID, int32 NewStock, int32 NewPrice);

	const TArray<FShopItemState>& GetAllShopItems() const { return ShopItems; }
	FShopItemState* GetShopItem(int32 ItemID);

	bool ShopItemRuleCheck(const FShopItemState* Item, int32 Quantity) const;
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_ShopItems();
};
