// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkillComponent.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "SkillDomain.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	MaxSkillSlots = 8;
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USkillComponent, SkillSlots);
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeMappers();
}

void USkillComponent::InitializeMappers()
{
	// TODO: Initialize mappers from subsystem or dependency injection
	// DtoMapper = GetWorld()->GetSubsystem<USkillSubsystem>()->GetDtoMapper();
	// AssetMapper = GetWorld()->GetSubsystem<USkillSubsystem>()->GetAssetMapper();
	// ModelBuilder = GetWorld()->GetSubsystem<USkillSubsystem>()->GetModelBuilder();
}

bool USkillComponent::ValidateMappers() const
{
	return DtoMapper.GetInterface() && AssetMapper.GetInterface() && ModelBuilder.GetInterface();
}

// ========================================================================
// AGGREGATE ROOT METHODS - BUSINESS LOGIC WITH INVARIANT PROTECTION
// ========================================================================

bool USkillComponent::RegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData)
{
	if (!CanRegisterSkill(SlotIndex, SkillData))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register skill at SlotIndex %d"), SlotIndex);
		return false;
	}

	// Í∏∞Ï°¥ ?¨Î°Ø ?ïÏû• ?êÎäî ?ùÏÑ±
	while (SkillSlots.Num() <= SlotIndex)
	{
		USkillSlot* NewSlot = NewObject<USkillSlot>(this);
		int32 NewSlotIndex = SkillSlots.Num();
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), NewSlotIndex);
		NewSlot->Initialize(NewSlotIndex, SlotKey, nullptr);
		SkillSlots.Add(NewSlot);
	}

	// ?¨Î°Ø???§ÌÇ¨ ?§Ï†ï
	USkillSlot* TargetSlot = SkillSlots[SlotIndex];
	TargetSlot->SetSkillData(SkillData, SkillData->SkillID);

	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Registered skill '%s' to SlotIndex %d"), 
		*SkillData->GetName(), SlotIndex);

	// Domain Event Î∞úÌñâ
	OnSkillRegistered.Broadcast(SlotIndex);
	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();

	return true;
}

void USkillComponent::UnregisterSkill(int32 SlotIndex)
{
	if (!CanUnregisterSkill(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot unregister skill at SlotIndex %d"), SlotIndex);
		return;
	}

	USkillSlot* TargetSlot = SkillSlots[SlotIndex];
	if (TargetSlot)
	{
		TargetSlot->ClearSkill();
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Unregistered skill from SlotIndex %d"), SlotIndex);

		// Domain Event Î∞úÌñâ
		OnSkillUnregistered.Broadcast(SlotIndex);
		OnSkillsChanged.Broadcast();
		NotifySkillStateChanged();
	}
}

void USkillComponent::SwapSkills(int32 SlotIndexA, int32 SlotIndexB)
{
	if (!CanSwapSkills(SlotIndexA, SlotIndexB))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot swap skills between SlotIndex %d and %d"), 
			SlotIndexA, SlotIndexB);
		return;
	}

	if (SlotIndexA == SlotIndexB) return;

	USkillSlot* SlotA = SkillSlots[SlotIndexA];
	USkillSlot* SlotB = SkillSlots[SlotIndexB];

	if (SlotA && SlotB)
	{
		// ?ÑÏãúÎ°?SlotA???∞Ïù¥???Ä??
		USkillDataAsset* TempSkillData = SlotA->SkillData;
		int32 TempSkillId = SlotA->SkillId;

		// SlotB???∞Ïù¥?∞Î? SlotAÎ°?Î≥µÏÇ¨
		SlotA->SetSkillData(SlotB->SkillData, SlotB->SkillId);
		
		// ?ÑÏãú ?∞Ïù¥?∞Î? SlotBÎ°?Î≥µÏÇ¨
		SlotB->SetSkillData(TempSkillData, TempSkillId);

		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Swapped skills between SlotIndex %d and %d"), 
			SlotIndexA, SlotIndexB);

		// Domain Event Î∞úÌñâ
		OnSkillsSwapped.Broadcast(SlotIndexA, SlotIndexB);
		OnSkillsChanged.Broadcast();
		NotifySkillStateChanged();
	}
}

/*Not Used*/
void USkillComponent::Server_SetSkillSlots(const TArray<USkillSlot*>& InSkillSlots)
{
	SkillSlots.Empty();
	for (USkillSlot* Slot : InSkillSlots)
	{
		if (Slot)
		{
			SkillSlots.Add(Slot);
		}
	}
	
	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();
}

// ========================================================================
// DOMAIN LOGIC METHODS - BUSINESS RULES VALIDATION
// ========================================================================

bool USkillComponent::CanRegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData) const
{
	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot register null SkillData"));
		return false;
	}

	if (SlotIndex < 0 || SlotIndex >= MaxSkillSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Invalid slot index %d"), SlotIndex);
		return false;
	}

	// ?¥Î? Í∞ôÏ? ?§ÌÇ¨???±Î°ù?òÏñ¥ ?àÎäîÏßÄ ?ïÏù∏
	if (HasSkill(SkillData))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Skill '%s' is already registered"), 
			*SkillData->GetName());
		return false;
	}

	return true;
}

bool USkillComponent::CanUnregisterSkill(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SkillSlots.Num())
	{
		return false;
	}

	const USkillSlot* TargetSlot = SkillSlots[SlotIndex];
	return TargetSlot && !TargetSlot->IsEmpty();
}

bool USkillComponent::CanSwapSkills(int32 SlotIndexA, int32 SlotIndexB) const
{
	if (SlotIndexA < 0 || SlotIndexA >= SkillSlots.Num() ||
		SlotIndexB < 0 || SlotIndexB >= SkillSlots.Num())
	{
		return false;
	}

	return SlotIndexA != SlotIndexB;
}

bool USkillComponent::CanUpdateCooldown(int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown) const
{
	if (SlotIndex < 0 || SlotIndex >= SkillSlots.Num())
	{
		return false;
	}

	const USkillSlot* TargetSlot = SkillSlots[SlotIndex];
	return TargetSlot && !TargetSlot->IsEmpty();
}

bool USkillComponent::CanSaveSkills(const FSkillDomain& SkillData) const
{
	// ?ÑÎ©î???∞Ïù¥???†Ìö®??Í≤ÄÏ¶?
	return SkillData.SkillSlots.Num() <= MaxSkillSlots;
}

bool USkillComponent::HasSkill(USkillDataAsset* SkillData) const
{
	if (!SkillData)
	{
		return false;
	}

	for (const USkillSlot* Slot : SkillSlots)
	{
		if (Slot && Slot->SkillData == SkillData)
		{
			return true;
		}
	}

	return false;
}

bool USkillComponent::HasAvailableSlot() const
{
	return SkillSlots.Num() < MaxSkillSlots || GetAvailableSlotIndex() != -1;
}

int32 USkillComponent::GetAvailableSlotIndex() const
{
	for (int32 i = 0; i < SkillSlots.Num(); ++i)
	{
		if (SkillSlots[i] && SkillSlots[i]->IsEmpty())
		{
			return i;
		}
	}

	// ?àÎ°ú???¨Î°Ø??Ï∂îÍ??????àÎäî Í≤ΩÏö∞
	if (SkillSlots.Num() < MaxSkillSlots)
	{
		return SkillSlots.Num();
	}

	return -1;
}

// ========================================================================
// QUERY METHODS - READ-ONLY ACCESS
// ========================================================================

USkillSlot* USkillComponent::GetSkillSlotByIndex(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < SkillSlots.Num())
	{
		return SkillSlots[SlotIndex];
	}
	return nullptr;
}

USkillSlot* USkillComponent::GetSkillSlotByKeyAndIndex(const FString& SlotKey, int32 SlotIndex) const
{
	USkillSlot* Slot = GetSkillSlotByIndex(SlotIndex);
	if (Slot && Slot->SlotKey == SlotKey)
	{
		return Slot;
	}
	return nullptr;
}

// ========================================================================
// 3-LAYER MAPPING INTEGRATION METHODS (Updated Implementation)
// ========================================================================

void USkillComponent::LoadSkillSlotsFromDatabase(int32 UserId, const FString& SlotKey)
{
	if (!ValidateMappers())
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComponent: Mappers not initialized"));
		return;
	}

	// TODO: Implement through SkillSubsystem coordination
	// This should be called by SkillDomainService, not directly
	UE_LOG(LogTemp, Warning, TEXT("SkillComponent::LoadSkillSlotsFromDatabase should be called through SkillSubsystem"));
}

void USkillComponent::SaveSkillSlotsToDatabase(int32 UserId)
{
	if (!ValidateMappers())
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComponent: Mappers not initialized"));
		return;
	}

	// TODO: Implement through SkillSubsystem coordination
	// This should be called by SkillDomainService, not directly
	UE_LOG(LogTemp, Warning, TEXT("SkillComponent::SaveSkillSlotsToDatabase should be called through SkillSubsystem"));
}

void USkillComponent::BuildSkillSlotsFromMappers(
	const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
	const TArray<USkillDataAsset*>& SkillDataAssets)
{
	if (!ValidateMappers())
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComponent: Mappers not initialized"));
		return;
	}

	SkillSlots.Empty();

	for (const FSkillSlotDatabaseDTO& SlotDTO : SlotDTOs)
	{
		// AssetMapperÎ•??µÌï¥ ?¥Îãπ?òÎäî SkillDataAsset Ï∞æÍ∏∞
		USkillDataAsset* MatchingAsset = nullptr;
		for (USkillDataAsset* Asset : SkillDataAssets)
		{
			if (Asset && Asset->SkillID == SlotDTO.SkillId)
			{
				MatchingAsset = Asset;
				break;
			}
		}

		// ModelBuilderÎ•??µÌï¥ Domain Model???ùÏÑ±?òÍ≥† USkillSlot?ºÎ°ú Î≥Ä??
		if (MatchingAsset && ModelBuilder.GetInterface())
		{
			FSkillDomainModel DomainModel = ModelBuilder.GetInterface()->BuildDomainModel(SlotDTO, MatchingAsset);
			USkillSlot* NewSlot = ModelBuilder.GetInterface()->BuildSkillSlotEntity(DomainModel);
			if (NewSlot)
			{
				// ?¨Î°Ø Î∞∞Ïó¥ ?¨Í∏∞ ?ïÏû•
				while (SkillSlots.Num() <= SlotDTO.SlotIndex)
				{
					USkillSlot* EmptySlot = NewObject<USkillSlot>(this);
					int32 EmptySlotIndex = SkillSlots.Num();
					FString EmptySlotKey = FString::Printf(TEXT("Slot_%d"), EmptySlotIndex);
					EmptySlot->Initialize(EmptySlotIndex, EmptySlotKey, nullptr);
					SkillSlots.Add(EmptySlot);
				}
				
				// ?ïÌôï???∏Îç±?§Ïóê ?¨Î°Ø ?§Ï†ï
				if (SlotDTO.SlotIndex >= 0 && SlotDTO.SlotIndex < SkillSlots.Num())
				{
					SkillSlots[SlotDTO.SlotIndex] = NewSlot;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillComponent: No matching SkillDataAsset found for SkillId %d"), SlotDTO.SkillId);
		}
	}

	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();
}

TArray<FSkillSlotDatabaseDTO> USkillComponent::ExtractDTOsFromSkillSlots(const FString& UserId) const
{
	TArray<FSkillSlotDatabaseDTO> DTOs;

	if (!ValidateMappers())
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComponent: Mappers not initialized"));
		return DTOs;
	}

	for (int32 i = 0; i < SkillSlots.Num(); ++i)
	{
		const USkillSlot* Slot = SkillSlots[i];
		if (Slot && !Slot->IsEmpty() && ModelBuilder.GetInterface())
		{
			FSkillSlotDatabaseDTO DTO = ModelBuilder.GetInterface()->ExtractSlotDTO(Slot);
			DTO.UserId = UserId; // UserIdÎ•?Í∑∏Î?Î°??†Îãπ
			DTOs.Add(DTO);
		}
	}

	return DTOs;
}

// ========================================================================
// LEGACY DOMAIN INTEGRATION METHODS - DEPRECATED
// ========================================================================

UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: BuildSkillSlotsFromMappers() instead")
void USkillComponent::SyncWithDomain(const FSkillDomain& SkillData)
{
	UE_LOG(LogTemp, Warning, TEXT("SkillComponent::SyncWithDomain is deprecated. Use 3-Layer Mapping Architecture instead."));
	
	// ?àÍ±∞???∏Ìôò?±ÏùÑ ?ÑÌïú ?ÑÏãú Íµ¨ÌòÑ
	SkillSlots.Empty();

	for (const FSkillSlotDTO& SlotDTO : SkillData.SkillSlots)
	{
		USkillSlot* NewSlot = NewObject<USkillSlot>(this);
		
		// Í∏∞Î≥∏ Ï¥àÍ∏∞??
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), SlotDTO.SlotIndex);
		NewSlot->Initialize(SlotDTO.SlotIndex, SlotKey, nullptr);
		
		// ?àÍ±∞??DTO?êÏÑú Í∏∞Î≥∏ ?ïÎ≥¥Îß??§Ï†ï
		NewSlot->SlotIndex = SlotDTO.SlotIndex;
		NewSlot->SkillId = SlotDTO.SkillID;
		NewSlot->LastUsedTime = SlotDTO.LastUsedTime;
		
		SkillSlots.Add(NewSlot);
	}

	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();
}

UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: ExtractDTOsFromSkillSlots() instead")
FSkillDomain USkillComponent::ExtractDomain() const
{
	UE_LOG(LogTemp, Warning, TEXT("SkillComponent::ExtractDomain is deprecated. Use 3-Layer Mapping Architecture instead."));
	
	FSkillDomain DomainData;
	DomainData.PlayerId = FGuid(); // TODO: Get actual player ID

	for (const USkillSlot* Slot : SkillSlots)
	{
		if (Slot && !Slot->IsEmpty())
		{
			FSkillSlotDTO SlotDTO;
			SlotDTO.SlotIndex = Slot->SlotIndex;
			SlotDTO.SkillID = Slot->SkillId;
			SlotDTO.LastUsedTime = Slot->LastUsedTime;
			SlotDTO.bIsActive = true;
			SlotDTO.RemainingCooldown = 0.0f; // Í≥ÑÏÇ∞??Í∞íÏù¥ ?ÑÏöî?òÎ©¥ Î≥ÑÎèÑ Í≥ÑÏÇ∞ ?ÑÏöî
			
			DomainData.SkillSlots.Add(SlotDTO);
		}
	}

	return DomainData;
}

// ========================================================================
// REPLICATION AND NOTIFICATION
// ========================================================================

void USkillComponent::OnRep_SkillSlots()
{
	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();
}

void USkillComponent::NotifySkillStateChanged()
{
	// TArray<USkillSlot*> ?ïÌÉúÎ°?Î≥Ä?òÌïò???¥Î≤§??Î∞úÌñâ
	TArray<USkillSlot*> SlotArray;
	for (USkillSlot* Slot : SkillSlots)
	{
		SlotArray.Add(Slot);
	}
	
	OnSkillStateChanged.Broadcast(SlotArray);
}