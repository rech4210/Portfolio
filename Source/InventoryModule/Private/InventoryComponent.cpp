// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Items);
}

void UInventoryComponent::AddItem(TSubclassOf<UItemDataAsset> ItemDataClass, int32 Quantity)
{
	if (GetOwnerRole() < ROLE_Authority || !ItemDataClass || Quantity <= 0)
	{
		return;
	}

	// TODO: Add logic for stacking with existing items

	UFInventoryItem* NewItem = NewObject<UFInventoryItem>();
	NewItem->ItemData =  GetDefault<UItemDataAsset>(ItemDataClass);
	NewItem->Quantity = Quantity;
	for (auto it : Items) {
		// 수량 체크를 따로 함수로 + max 제한까지 stacking
		// do operation override here.
		 if (it->ItemData == NewItem->ItemData && (it->ItemData->MaxStackCount > (it->Quantity + NewItem->Quantity))) {
		 	it->Quantity += NewItem->Quantity;
		 }
		 else {
		 	Items.Add(NewItem);
		 }
	}

	OnRep_Items();
}

void UInventoryComponent::RemoveItem(TSubclassOf<UItemDataAsset> ItemDataClass, int32 Quantity)
{
	if (GetOwnerRole() < ROLE_Authority || !ItemDataClass || Quantity <= 0)
	{
		return;
	}

	// TODO: Implement actual item removal logic (finding the item, reducing quantity, etc.)
}

bool UInventoryComponent::HasItem(const UItemDataAsset* ItemToSell, int Quantity) const {
	for (auto item : Items) {
		if (item->ItemData == ItemToSell) {
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::HasEnoughSpace(const UItemDataAsset* ItemToBuy, int I) {
	//check Items array's remain place for add item. if (I > MaxStackCount) -> add new stack area
	return true;
}

void UInventoryComponent::OnRep_Items()
{
	// TODO: UI 바인딩 - Broadcast OnInventoryChanged delegate here
}
