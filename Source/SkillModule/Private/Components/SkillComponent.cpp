#include "Components/SkillComponent.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include "SkillSubsystem.h"
#include "Engine/World.h"
#include "SkillDomain.h"

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
	
	// 클라이언트에서 복제된 데이터를 받았을 때 SkillSubsystem에 알림
	// if (auto* SkillSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USkillSubsystem>())
	// {
	// 	SkillSubsystem->Client_OnSkillStateUpdated(this);
	// }
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
	const TObjectPtr<USkillSlot>* FoundSlot = SkillSlots.FindByPredicate([&](const USkillSlot* Slot)
	{
		return Slot && Slot->SlotId == SlotId;
	});

	return FoundSlot ? *FoundSlot : nullptr;
}	

FGuid USkillComponent::GetSkillSlotGuidByIndex(int32 index) const {
	if (index > MaxSkillSlots || SkillSlots.IsEmpty()) {
		return  FGuid();
	}
	return SkillSlots[index]->SlotId;
}

// ========================================================================
// DOMAIN LOGIC METHODS - BUSINESS RULES VALIDATION
// ========================================================================

bool USkillComponent::CanRegisterSkill(USkillDataAsset* SkillData) const
{
	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register null SkillData"));
		return false;
	}

	// Business rule: Cannot exceed max slots
	if (SkillSlots.Num() >= MaxSkillSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register skill - max slots reached"));
		return false;
	}

	// Business rule: Cannot register duplicate skills
	if (HasSkill(SkillData))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register skill - already registered"));
		return false;
	}

	return true;
}

bool USkillComponent::CanUnregisterSkill(const FGuid& SlotId) const
{
	if (!SlotId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot unregister skill - invalid SlotId"));
		return false;
	}

	// Business rule: Slot must exist
	USkillSlot* FoundSlot = GetSkillSlotByGuid(SlotId);
	if (!FoundSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot unregister skill - slot not found"));
		return false;
	}

	// Business rule: Could add additional checks here (e.g., skill not in use)
	return true;
}

bool USkillComponent::CanSwapSkills(const FGuid& SlotIdA, const FGuid& SlotIdB) const
{
	if (!SlotIdA.IsValid() || !SlotIdB.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot swap skills - invalid SlotIds"));
		return false;
	}

	if (SlotIdA == SlotIdB)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot swap skills - same slot"));
		return false;
	}

	// Business rule: Both slots must exist
	USkillSlot* SlotA = GetSkillSlotByGuid(SlotIdA);
	USkillSlot* SlotB = GetSkillSlotByGuid(SlotIdB);

	if (!SlotA || !SlotB)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot swap skills - one or both slots not found"));
		return false;
	}

	// Business rule: Could add additional checks here (e.g., skills not in use)
	return true;
}

bool USkillComponent::CanUpdateCooldown(const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown) const
{
	if (!SlotId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot update cooldown - invalid SlotId"));
		return false;
	}

	// Business rule: Slot must exist
	USkillSlot* FoundSlot = GetSkillSlotByGuid(SlotId);
	if (!FoundSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot update cooldown - slot not found"));
		return false;
	}

	// Business rule: Remaining cooldown cannot be negative
	if (RemainingCooldown < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot update cooldown - negative remaining cooldown"));
		return false;
	}

	// Business rule: Last used time should not be in the future
	if (LastUsedTime > FDateTime::Now())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot update cooldown - last used time in future"));
		return false;
	}

	return true;
}

bool USkillComponent::CanSaveSkills(const FSkillDomain& SkillData) const
{
	// Business rule: Skill data must be valid
	if (!SkillData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot save skills - invalid SkillData"));
		return false;
	}

	// Business rule: Cannot exceed max slots
	if (SkillData.SkillSlots.Num() > MaxSkillSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot save skills - exceeds max slots"));
		return false;
	}

	// Business rule: All skill slots must have valid IDs
	for (const auto& Slot : SkillData.SkillSlots)
	{
		if (!Slot.SlotId.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot save skills - invalid slot ID found"));
			return false;
		}
	}

	return true;
}

bool USkillComponent::HasSkill(USkillDataAsset* SkillData) const
{
	if (!SkillData)
	{
		return false;
	}

	return SkillSlots.ContainsByPredicate([SkillData](const USkillSlot* Slot)
	{
		return Slot && Slot->SkillData && Slot->SkillData->SkillID == SkillData->SkillID;
	});
}

bool USkillComponent::HasAvailableSlot() const
{
	return SkillSlots.Num() < MaxSkillSlots;
}

int32 USkillComponent::GetAvailableSlotIndex() const
{
	if (SkillSlots.Num() >= MaxSkillSlots)
	{
		return -1;
	}
	return SkillSlots.Num(); // Next available index
}

// ========================================================================
// DOMAIN INTEGRATION METHODS - AGGREGATE SYNCHRONIZATION
// ========================================================================

void USkillComponent::SyncWithDomain(const FSkillDomain& SkillData)
{
	// Clear current slots
	SkillSlots.Empty();

	// Recreate slots from domain data
	for (const auto& SlotDTO : SkillData.SkillSlots)
	{
		USkillSlot* NewSlot = NewObject<USkillSlot>(this);
		
		// Find skill data asset by ID (you might need to implement a skill data asset lookup)
		// For now, we'll create a basic slot
		NewSlot->SlotId = SlotDTO.SlotId;
		NewSlot->LastUsedTime = SlotDTO.LastUsedTime;
		
		// TODO: Lookup SkillData asset by SkillID from SlotDTO.SkillID
		// NewSlot->SkillData = FindSkillDataByID(SlotDTO.SkillID);
		
		SkillSlots.Add(NewSlot);
	}

	// Notify of changes
	NotifySkillStateChanged();
	
	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Synchronized with domain data - %d slots"), SkillSlots.Num());
}

FSkillDomain USkillComponent::ExtractDomain() const
{
	FSkillDomain DomainData;
	DomainData.PlayerId = GetOwner() ? GetOwner()->GetUniqueID() : 0;

	// Convert component slots to domain DTOs
	for (const USkillSlot* Slot : SkillSlots)
	{
		if (Slot)
		{
			FSkillSlotDTO SlotDTO;
			SlotDTO.SlotId = Slot->SlotId;
			SlotDTO.SkillID = Slot->SkillData ? Slot->SkillData->SkillID : -1;
			SlotDTO.LastUsedTime = Slot->LastUsedTime;
			
			DomainData.SkillSlots.Add(SlotDTO);
		}
	}

	return DomainData;
}

void USkillComponent::Server_SetSkillSlots(const TArray<USkillSlot*>& InSkillSlots)
{
	// Server-side method to update skill slots (used by Repository/DomainService)
	SkillSlots.Empty();
	for (USkillSlot* Slot : InSkillSlots)
	{
		if (Slot)
		{
			SkillSlots.Add(Slot);
		}
	}
	
	// Mark for replication
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		GetOwner()->ForceNetUpdate();
	}
	
	NotifySkillStateChanged();
}

void USkillComponent::NotifySkillStateChanged()
{
	OnSkillsChanged.Broadcast();
	OnSkillStateChanged.Broadcast(SkillSlots);
}