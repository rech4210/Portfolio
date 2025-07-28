// @Needmodifi
// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "EquipmentSubsystem.h"
#include "Engine/World.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipmentComponent, EquipmentSlots);
}

void UEquipmentComponent::OnRep_EquipmentSlots()
{
	// ?¥ë¹„ ?íƒœê°€ ë³€ê²½ë˜?ˆìŒ???Œë¦¼
	OnEquipmentStateChanged.Broadcast(EquipmentSlots);
	
	// ?´ë¼?´ì–¸?¸ì—??ë³µì œ???°ì´?°ë? ë°›ì•˜????EquipmentSubsystem???Œë¦¼
	if (auto* EquipmentSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UEquipmentSubsystem>())
	{
		EquipmentSubsystem->Client_OnEquipmentStateUpdated(this);
	}
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UEquipmentComponent::EquipItem(const FEquipmentSlotState& EquipmentState)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentComponent: EquipItem can only be called on server authority"));
		return false;
	}

	if (EquipmentSlots.Num() >= MaxEquipmentSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentComponent: Cannot equip item, max slots reached"));
		return false;
	}

	// ?´ë? ?´ë‹¹ ?¬ë¡¯???¥ë¹„ê°€ ?ˆëŠ”ì§€ ?•ì¸
	if (GetEquipmentSlot(EquipmentState.SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentComponent: Slot %d is already occupied"), EquipmentState.SlotIndex);
		return false;
	}

	EquipmentSlots.Add(EquipmentState);
	UE_LOG(LogTemp, Log, TEXT("EquipmentComponent: Equipped item %d in slot %d"), EquipmentState.ItemID, EquipmentState.SlotIndex);
	return true;
}

bool UEquipmentComponent::UnequipItem(int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentComponent: UnequipItem can only be called on server authority"));
		return false;
	}

	const int32 RemovedCount = EquipmentSlots.RemoveAll([SlotIndex](const FEquipmentSlotState& Equipment)
	{
		return Equipment.SlotIndex == SlotIndex;
	});

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("EquipmentComponent: Unequipped item from slot %d"), SlotIndex);
		return true;
	}

	return false;
}

bool UEquipmentComponent::UpdateEquipmentEnhancement(int32 SlotIndex, const TArray<int32>& NewEnhancementLevel)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentComponent: UpdateEquipmentEnhancement can only be called on server authority"));
		return false;
	}

	if (FEquipmentSlotState* Equipment = GetEquipmentSlot(SlotIndex))
	{
		Equipment->EnhancementLevel = NewEnhancementLevel;
		UE_LOG(LogTemp, Log, TEXT("EquipmentComponent: Updated enhancement for slot %d"), SlotIndex);
		return true;
	}

	return false;
}

FEquipmentSlotState* UEquipmentComponent::GetEquipmentSlot(int32 SlotIndex)
{
	return EquipmentSlots.FindByPredicate([SlotIndex](const FEquipmentSlotState& Equipment)
	{
		return Equipment.SlotIndex == SlotIndex;
	});
}
