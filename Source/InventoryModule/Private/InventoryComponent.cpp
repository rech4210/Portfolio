// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "InventorySubsystem.h"
#include "GameFramework/PlayerState.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Only replicate the inventory to the owning client
	DOREPLIFETIME_CONDITION(UInventoryComponent, Items, COND_OwnerOnly);
}

void UInventoryComponent::Server_SetInventoryItems(const TArray<UFInventoryItem*>& InItems)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Items.Empty();
		for (UFInventoryItem* Item : InItems)
		{
			if (Item)
			{
				// The component now takes ownership of the item objects.
				Items.Add(Item);
			}
		}
		// For the server, we can directly call the OnRep function or the delegate.
		// This ensures server-side logic that depends on the inventory state is also executed.
		OnRep_Items();
	}
}

void UInventoryComponent::OnRep_Items()
{
	// This is called on the client when the 'Items' array is replicated.
	// We notify the subsystem to handle any client-side logic.
	if (GetOwner())
	{
		if (UInventorySubsystem* InventorySubsystem = GetOwner()->GetGameInstance()->GetSubsystem<UInventorySubsystem>())
		{
			InventorySubsystem->Client_OnInventoryUpdated(this);
		}
	}

	// Broadcast the delegate for UI updates.
	OnInventoryUpdated.Broadcast();
}

bool UActorComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    for (UFInventoryItem* Item : Items)
    {
        if (Item)
        {
            bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
        }
    }

    return bWroteSomething;
}