#include "Components/SkillComponent.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USkillComponent, SkillSlots);
}

void USkillComponent::OnRep_SkillSlots()
{
	// 스킬 상태가 변경되었음을 알림 (현재 전체 스킬 목록과 함께)
	OnSkillStateChanged.Broadcast(SkillSlots);
}

int32 USkillComponent::GetMaxSlotCount() const {
	return MaxSkillSlots;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool USkillComponent::RegisterSkill(USkillDataAsset* SkillData)
{
	if (!SkillData || !SkillData->AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register skill with null SkillData or AbilityClass."));
		return false;
	}
    
	if (SkillSlots.Num() >= MaxSkillSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register new skill '%s', max slots reached."), *SkillData->GetName());
		return false;
	}

	USkillSlot* NewSlot = NewObject<USkillSlot>(this);
	NewSlot->Initialize(SkillData, SkillData->AbilityClass);
	SkillSlots.Add(NewSlot);

	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Registered skill '%s' with SlotId %s"), *SkillData->GetName(), *NewSlot->SlotId.ToString());

	return true;
}

void USkillComponent::UnregisterSkill(const FGuid& SlotId)
{
	const int32 RemovedCount = SkillSlots.RemoveAll([&](const USkillSlot* Slot)
	{
		return Slot && Slot->SlotId == SlotId;
	});

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Unregistered skill with SlotId %s"), *SlotId.ToString());
	}
}

void USkillComponent::SwapSkills(const FGuid& SlotIdA, const FGuid& SlotIdB)
{
	if (SlotIdA == SlotIdB) return;

	int32 IndexA = SkillSlots.IndexOfByPredicate([&](const USkillSlot* Slot) { return Slot && Slot->SlotId == SlotIdA; });
	int32 IndexB = SkillSlots.IndexOfByPredicate([&](const USkillSlot* Slot) { return Slot && Slot->SlotId == SlotIdB; });

	if (IndexA != INDEX_NONE && IndexB != INDEX_NONE)
	{
		SkillSlots.Swap(IndexA, IndexB);
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Swapped skills with SlotIds %s and %s"), *SlotIdA.ToString(), *SlotIdB.ToString());
	}
}

USkillSlot* USkillComponent::GetSkillSlotByGuid(const FGuid& SlotId) const
{
	USkillSlot* const* FoundSlot = SkillSlots.FindByPredicate([&](const USkillSlot* Slot)
	{
		return Slot && Slot->SlotId == SlotId;
	});

	return FoundSlot ? *FoundSlot : nullptr;
}

FGuid USkillComponent::GetSkillSlotGuidByIndex(int32 index) const {
	return SkillSlots[index]->SlotId;
}

//
// void USkillComponent::MarkUsed(const FGuid& SlotId, const FDateTime& Timestamp)
// {
// 	USkillSlot* Slot = GetSkillSlot(SlotId);
// 	if (Slot && Slot->SkillData)
// 	{
// 		Slot->SetLastUsedTime(Timestamp);
// 		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Marked skill '%s' as used. SlotId: %s, Timestamp: %s"), *Slot->SkillData->GetName(), *SlotId.ToString(), *Timestamp.ToString());
// 	}
// } 