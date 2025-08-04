
#include "Components/SkillComponent.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "SkillDomain.h"
#include "SkillSubsystem.h"
#include "Engine/Engine.h"
#include "Interface/IClientComponentProvider.h"
#include "Mappers/SkillAssetMapper.h"
#include "Mappers/SkillModelBuilder.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameSharedModule/Public/Interface/IClientComponentProvider.h"

// ========================================================================
// REPLICATION STRUCT IMPLEMENTATIONS
// ========================================================================

void FSkillSlotReplicationArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	if (Owner)
	{
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: %d skill slots ADDED to replication (FinalSize: %d)"), 
			AddedIndices.Num(), FinalSize);
		
		for (int32 Index : AddedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				const FSkillSlotReplicationData& SlotData = Items[Index].SlotData;
				UE_LOG(LogTemp, Log, TEXT("  - Added Slot[%d]: SkillId=%d, Name='%s'"), 
					SlotData.SlotIndex, SlotData.SkillId, *SlotData.SkillName);
			}
		}
	}
}

void FSkillSlotReplicationArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	if (Owner)
	{
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: %d skill slots CHANGED in replication (FinalSize: %d)"), 
			ChangedIndices.Num(), FinalSize);
		
		for (int32 Index : ChangedIndices)
		{
			if (Items.IsValidIndex(Index))
			{
				const FSkillSlotReplicationData& SlotData = Items[Index].SlotData;
				UE_LOG(LogTemp, Log, TEXT("  - Changed Slot[%d]: SkillId=%d, Name='%s'"), 
					SlotData.SlotIndex, SlotData.SkillId, *SlotData.SkillName);
			}
		}
	}
}

void FSkillSlotReplicationArray::PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	if (Owner)
	{
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: %d skill slots REMOVED from replication (FinalSize: %d)"), 
			RemovedIndices.Num(), FinalSize);
	}
}

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	MaxSkillSlots = 8;
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USkillComponent, SkillSlotsReplication);
	
	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Registered SkillSlotsReplication for replication"));
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeMappers();
	
	SkillSlotsReplication.SetOwner(this);
	
	if (GetOwner()->HasAuthority())
	{
		InitializeEmptySlots();
	}
}

void USkillComponent::InitializeMappers()
{
	DtoMapper = NewObject<USkillDtoMapper>(this, TEXT("SkillDtoMapper"));
	AssetMapper = NewObject<USkillAssetMapper>(this, TEXT("SkillAssetMapper"));
	ModelBuilder = NewObject<USkillModelBuilder>(this, TEXT("SkillModelBuilder"));
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

	while (SkillSlotsReplication.Items.Num() <= SlotIndex)
	{
		FSkillSlotReplicationData NewSlotData;
		int32 NewSlotIndex = SkillSlotsReplication.Items.Num();
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), NewSlotIndex);
		NewSlotData.Initialize(NewSlotIndex, SlotKey, nullptr);
		
		FSkillSlotReplicationItem NewItem(NewSlotData);
		SkillSlotsReplication.Items.Add(NewItem);
	}
	
	FSkillSlotReplicationData* TargetSlot = GetMutableSkillSlotDataByIndex(SlotIndex);
	if (TargetSlot)
	{
		TargetSlot->SetSkillData(SkillData, SkillData->SkillID);
	}

	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Registered skill '%s' to SlotIndex %d"), 
		*SkillData->GetName(), SlotIndex);

	OnSkillRegistered.Broadcast(SlotIndex);
	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();
	
	MarkSlotForReplication(SlotIndex);

	return true;
}

void USkillComponent::UnregisterSkill(int32 SlotIndex)
{
	if (!CanUnregisterSkill(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Cannot unregister skill at SlotIndex %d"), SlotIndex);
		return;
	}

	FSkillSlotReplicationData* TargetSlot = GetMutableSkillSlotDataByIndex(SlotIndex);
	if (TargetSlot)
	{
		TargetSlot->ClearSkill();
		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Unregistered skill from SlotIndex %d"), SlotIndex);

		OnSkillUnregistered.Broadcast(SlotIndex);
		OnSkillsChanged.Broadcast();
		NotifySkillStateChanged();
		
		MarkSlotForReplication(SlotIndex);
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

	FSkillSlotReplicationData* SlotA = GetMutableSkillSlotDataByIndex(SlotIndexA);
	FSkillSlotReplicationData* SlotB = GetMutableSkillSlotDataByIndex(SlotIndexB);

	if (SlotA && SlotB)
	{
		USkillDataAsset* TempSkillData = SlotA->SkillData;
		int32 TempSkillId = SlotA->SkillId;
		FString TempSkillName = SlotA->SkillName;
		FString TempSkillDescription = SlotA->SkillDescription;
		float TempCooldown = SlotA->Cooldown;

		SlotA->SetSkillData(SlotB->SkillData, SlotB->SkillId);
		SlotB->SetSkillData(TempSkillData, TempSkillId);

		UE_LOG(LogTemp, Log, TEXT("SkillComponent: Swapped skills between SlotIndex %d and %d"), 
			SlotIndexA, SlotIndexB);

		OnSkillsSwapped.Broadcast(SlotIndexA, SlotIndexB);
		OnSkillsChanged.Broadcast();
		NotifySkillStateChanged();
		
		MarkSlotForReplication(SlotIndexA);
		MarkSlotForReplication(SlotIndexB);
	}
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
	if (SlotIndex < 0 || SlotIndex >= MaxSkillSlots)
	{
		return false;
	}

	const FSkillSlotReplicationData* TargetSlot = GetSkillSlotDataByIndex(SlotIndex);
	return TargetSlot && !TargetSlot->IsEmpty();
}

bool USkillComponent::CanSwapSkills(int32 SlotIndexA, int32 SlotIndexB) const
{
	if (SlotIndexA < 0 || SlotIndexA >= MaxSkillSlots ||
		SlotIndexB < 0 || SlotIndexB >= MaxSkillSlots)
	{
		return false;
	}

	return SlotIndexA != SlotIndexB;
}

bool USkillComponent::HasSkill(USkillDataAsset* SkillData) const
{
	if (!SkillData)
	{
		return false;
	}

	for (const FSkillSlotReplicationItem& Item : SkillSlotsReplication.Items)
	{
		if (Item.SlotData.SkillData == SkillData)
		{
			return true;
		}
	}

	return false;
}

// ========================================================================
// QUERY METHODS - READ-ONLY ACCESS
// ========================================================================

const FSkillSlotReplicationData* USkillComponent::GetSkillSlotDataByIndex(int32 SlotIndex) const
{
	for (const FSkillSlotReplicationItem& Item : SkillSlotsReplication.Items)
	{
		if (Item.SlotData.SlotIndex == SlotIndex)
		{
			return &Item.SlotData;
		}
	}
	return nullptr;
}

FSkillSlotReplicationData* USkillComponent::GetMutableSkillSlotDataByIndex(int32 SlotIndex)
{
	for (FSkillSlotReplicationItem& Item : SkillSlotsReplication.Items)
	{
		if (Item.SlotData.SlotIndex == SlotIndex)
		{
			return &Item.SlotData;
		}
	}
	return nullptr;
}

const FSkillSlotReplicationData* USkillComponent::GetSkillSlotDataByKeyAndIndex(const FString& SlotKey, int32 SlotIndex) const
{
	const FSkillSlotReplicationData* SlotData = GetSkillSlotDataByIndex(SlotIndex);
	if (SlotData && SlotData->SlotKey == SlotKey)
	{
		return SlotData;
	}
	return nullptr;
}

TArray<FSkillSlotReplicationData> USkillComponent::GetAllSkillSlotsData() const
{
	TArray<FSkillSlotReplicationData> Result;
	for (const FSkillSlotReplicationItem& Item : SkillSlotsReplication.Items)
	{
		Result.Add(Item.SlotData);
	}
	return Result;
}

// ========================================================================
// 3-LAYER MAPPING INTEGRATION METHODS
// ========================================================================

void USkillComponent::BuildSkillSlotsFromMappers(
	const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
	const TArray<USkillDataAsset*>& SkillDataAssets)
{
	if (!ValidateMappers())
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComponent: Mappers not initialized"));
		return;
	}

	SkillSlotsReplication.Items.Empty();

	for (const FSkillSlotDatabaseDTO& SlotDTO : SlotDTOs)
	{
		USkillDataAsset* MatchingAsset = nullptr;
		for (USkillDataAsset* Asset : SkillDataAssets)
		{
			if (Asset && Asset->SkillID == SlotDTO.SkillId)
			{
				MatchingAsset = Asset;
				break;
			}
		}

		if (MatchingAsset && ModelBuilder.GetInterface())
		{
			FSkillDomainModel DomainModel = ModelBuilder.GetInterface()->BuildDomainModel(SlotDTO, MatchingAsset);
			FSkillSlotReplicationData NewSlotData = ModelBuilder.GetInterface()->BuildSkillSlotData(DomainModel);
			
			while (SkillSlotsReplication.Items.Num() <= SlotDTO.SlotIndex)
			{
				FSkillSlotReplicationData EmptySlotData;
				int32 EmptySlotIndex = SkillSlotsReplication.Items.Num();
				FString EmptySlotKey = FString::Printf(TEXT("Slot_%d"), EmptySlotIndex);
				EmptySlotData.Initialize(EmptySlotIndex, EmptySlotKey, nullptr);
				
				FSkillSlotReplicationItem EmptyItem(EmptySlotData);
				SkillSlotsReplication.Items.Add(EmptyItem);
			}
			
			if (SlotDTO.SlotIndex >= 0 && SlotDTO.SlotIndex < SkillSlotsReplication.Items.Num())
			{
				NewSlotData.SkillData = MatchingAsset;
				SkillSlotsReplication.Items[SlotDTO.SlotIndex].SlotData = NewSlotData;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillComponent: No matching SkillDataAsset found for SkillId %d"), SlotDTO.SkillId);
		}
	}

	OnSkillsChanged.Broadcast();
	NotifySkillStateChanged();
	
	SyncAllSlotsToReplication();
}

TArray<FSkillSlotDatabaseDTO> USkillComponent::ExtractDTOsFromSkillSlots(const FString& UserId) const
{
	TArray<FSkillSlotDatabaseDTO> DTOs;

	if (!ValidateMappers())
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComponent: Mappers not initialized"));
		return DTOs;
	}

	for (const FSkillSlotReplicationItem& Item : SkillSlotsReplication.Items)
	{
		const FSkillSlotReplicationData& SlotData = Item.SlotData;
		if (!SlotData.IsEmpty() && ModelBuilder.GetInterface())
		{
			FSkillSlotDatabaseDTO DTO = ModelBuilder.GetInterface()->ExtractSlotDTO(SlotData);
			DTO.UserId = UserId;
			DTOs.Add(DTO);
		}
	}

	return DTOs;
}

// ========================================================================
// REPLICATION AND NOTIFICATION
// ========================================================================

// ========================================================================
// SKILL REPLICATION METHODS
// ========================================================================

void USkillComponent::OnRep_SkillSlotsReplication()
{
	UE_LOG(LogTemp, Log, TEXT("SkillComponent: UI replication data updated with %d items"), 
		SkillSlotsReplication.Items.Num());
	
	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: OnRep called on server - this should not happen"));
		return;
	}

	if (APlayerState* OwnerPlayerState = Cast<APlayerState>(GetOwner()))
	{
		if (APlayerController* PC = OwnerPlayerState->GetPlayerController())
		{
			if (IClientManagerInterface* ClientManager = Cast<IClientManagerInterface>(PC))
			{
				ClientManager->SkillHUDReplication(SkillSlotsReplication);
				UE_LOG(LogTemp, Log, TEXT("SkillComponent: Successfully triggered UI update chain through PlayerController"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SkillComponent: PlayerController does not implement IClientManagerInterface"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Could not get PlayerController from PlayerState"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Owner is not a PlayerState"));
	}
	
	NotifySkillStateChanged();
}

void USkillComponent::MarkSlotForReplication(int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: MarkSlotForReplication called on client. SlotIndex: %d"), SlotIndex);
		return;
	}
	
	if (SlotIndex < 0 || SlotIndex >= MaxSkillSlots) 
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Invalid SlotIndex %d for replication"), SlotIndex);
		return;
	}
	
	for (int32 i = 0; i < SkillSlotsReplication.Items.Num(); ++i)
	{
		if (SkillSlotsReplication.Items[i].SlotData.SlotIndex == SlotIndex)
		{
			SkillSlotsReplication.MarkItemDirty(SkillSlotsReplication.Items[i]);
			UE_LOG(LogTemp, Log, TEXT("SkillComponent: Marked slot %d for replication (SkillId: %d)"), 
				SlotIndex, SkillSlotsReplication.Items[i].SlotData.SkillId);
			return;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("SkillComponent: Slot %d not found for replication marking"), SlotIndex);
}

void USkillComponent::SyncAllSlotsToReplication()
{
	if (!GetOwner()->HasAuthority()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: SyncAllSlotsToReplication called on client"));
		return;
	}
	
	for (auto& Item : SkillSlotsReplication.Items)
	{
		SkillSlotsReplication.MarkItemDirty(Item);
	}
	
	SkillSlotsReplication.MarkArrayDirty();
	
	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Synced %d slots to replication system"), 
		SkillSlotsReplication.Items.Num());
}

void USkillComponent::NotifySkillStateChanged()
{
	TArray<USkillSlot*> SlotArray;
	OnSkillStateChanged.Broadcast(SlotArray);
}

void USkillComponent::InitializeEmptySlots()
{
	if (!GetOwner()->HasAuthority()) return;
	
	for (int32 i = SkillSlotsReplication.Items.Num(); i < MaxSkillSlots; ++i)
	{
		FSkillSlotReplicationData NewSlotData;
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), i);
		NewSlotData.Initialize(i, SlotKey, nullptr);
		
		FSkillSlotReplicationItem NewItem(NewSlotData);
		SkillSlotsReplication.Items.Add(NewItem);
	}
	
	SyncAllSlotsToReplication();
	
	UE_LOG(LogTemp, Log, TEXT("SkillComponent: Initialized %d empty skill slots"), MaxSkillSlots);
}