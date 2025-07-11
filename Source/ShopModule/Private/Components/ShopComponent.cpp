// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ShopComponent.h"
#include "Net/UnrealNetwork.h"
#include "ShopSubsystem.h"
#include "Engine/World.h"

UShopComponent::UShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UShopComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShopComponent, ShopItems);
}

void UShopComponent::OnRep_ShopItems()
{
	// 상점 상태가 변경되었음을 알림
	OnShopStateChanged.Broadcast(ShopItems);
	
	// 클라이언트에서 복제된 데이터를 받았을 때 ShopSubsystem에 알림
	if (auto* ShopSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UShopSubsystem>())
	{
		ShopSubsystem->Client_OnShopStateUpdated(this);
	}
}

FShopItemState UShopComponent::GetShopItem(int32 ItemID) {
	return nullptr;
}

void UShopComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UShopComponent::AddShopItem(const FShopItemState& ItemState) {
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: AddShopItem can only be called on server authority"));
		return false;
	}

	// 이미 존재하는 아이템인지 확인
	if (GetShopItem(ItemState.ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Item with ID %d already exists"), ItemState.ItemID);
		return false;
	}

	ShopItems.Add(ItemState);
	UE_LOG(LogTemp, Log, TEXT("ShopComponent: Added shop item with ID %d"), ItemState.ItemID);
	return true;
}

bool UShopComponent::RemoveShopItem(int32 ItemID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: RemoveShopItem can only be called on server authority"));
		return false;
	}

	const int32 RemovedCount = ShopItems.RemoveAll([ItemID](const FShopItemState& Item)
	{
		return Item.ItemID == ItemID;
	});

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ShopComponent: Removed shop item with ID %d"), ItemID);
		return true;
	}

	return false;
}


bool UShopComponent::UpdateShopItem(int32 ItemID, int32 NewStock, int32 NewPrice)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: UpdateItemStock can only be called on server authority"));
		return false;
	}

	if (FShopItemState* Item = GetShopItem(ItemID))
	{
		Item->Stock += NewStock;
		Item->Price += NewPrice;
		UE_LOG(LogTemp, Log, TEXT("ShopComponent: Updated stock for item %d to %d"), ItemID, NewStock);
		return true;
	}

	return false;
}

bool UShopComponent::ShopItemRuleCheck(const FShopItemState Item, int32 Quantity) const {
	// if (!Item || !Item->CurrentCount > 0) {
	// 	return false;
	// }
	// // 개수 판단은 클라이언트 단에서 현재 수 이상으로 살 수 없도록 제한.
	// if (!Item->CurrentCount >= Quantity) {
	// 	return false;
	// }
	return true;
}
//
// UItemDataAsset* UShopComponent::GetShopItem(int32 ItemID)
// {
// 	return ShopItems.FindByPredicate([ItemID](const UItemDataAsset Item)
// 	{
// 		return Item->ItemID == ItemID;
// 	});
// }
