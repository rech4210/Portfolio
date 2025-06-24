// Copyright Epic Games, Inc. All Rights Reserved.

#include "Shared/Inventory/InventoryComponent.h"

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

void UInventoryComponent::AddItem(const FInventoryItem& Item)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		// Not the server, do nothing.
		return;
	}

	// TODO: Implement actual item adding logic (stacking, etc.)
	Items.Add(Item);

	// Manually call OnRep for the server to ensure UI updates
	OnRep_Items();
}

void UInventoryComponent::RemoveItem(FName ItemID, int32 Quantity)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	// TODO: Implement actual item removal logic
}

void UInventoryComponent::OnRep_Items()
{
	// TODO: UI 바인딩 - Broadcast OnInventoryChanged delegate here
} 