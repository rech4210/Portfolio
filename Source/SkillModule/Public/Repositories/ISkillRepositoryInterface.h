// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface/BaseRepositoryInterface.h"
#include "UObject/Interface.h"
#include "SkillDomain.h"
#include "Tasks/Task.h"
#include "ISkillRepositoryInterface.generated.h"

class USkillDataAsset;
class USkillComponent;
class APlayerState;

/**
 * 스킬의 정의(DataAsset)를 로드하기 위한 리포지토리 인터페이스입니다.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USkillConfigRepositoryInterface : public UBaseRepositoryInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillConfigRepositoryInterface : public IBaseRepositoryInterface
{
	GENERATED_BODY()

public:
	virtual void LoadSkillDefinitions(TArray<USkillDataAsset*>& OutSkillDefinitions) = 0;
};

/**
 * DDD-style repository interface for skill state management
 * Handles pure persistence operations without engine dependencies
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USkillRepositoryInterface : public UBaseRepositoryInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillRepositoryInterface : public IBaseRepositoryInterface
{
	GENERATED_BODY()

public:
	// ========================================================================
	// 3-LAYER MAPPING ARCHITECTURE METHODS (RECOMMENDED)
	// ========================================================================
	
	/**
	 * Load user skill slots using 3-layer mapping architecture
	 * @param UserId User ID to load skills for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @return Task returning skill slot DTOs
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> LoadUserSkillSlots(const FString& UserId, const FString& SlotKey) = 0;

	/**
	 * Save user skill slots using 3-layer mapping architecture
	 * @param UserId User ID to save skills for
	 * @param SkillSlotDTOs Skill slot DTOs to save
	 * @return Task returning success/failure result
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> SaveUserSkillSlots(const FString& UserId, const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs) = 0;

	/**
	 * Load skill master data using 3-layer mapping architecture
	 * @param SkillIds Optional list of specific skill IDs to load
	 * @return Task returning skill master data
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> LoadSkillMasterData(const TArray<int32>& SkillIds = {}) = 0;

	/**
	 * Update skill slot cooldown using 3-layer mapping architecture
	 * @param UserId User ID to update
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @param SlotIndex Slot index to update
	 * @param LastUsedTime When the skill was last used
	 * @return Task returning updated skill slots
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime) = 0;

	/**
	 * Clear all skill slots for a user using 3-layer mapping architecture
	 * @param UserId User ID to clear slots for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @return Task returning success/failure result
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> ClearUserSkillSlots(const FString& UserId, const FString& SlotKey) = 0;

	/**
	 * Get skill usage statistics using 3-layer mapping architecture
	 * @param UserId User ID to get stats for
	 * @param SkillId Specific skill ID (optional)
	 * @param StartDate Start date for statistics
	 * @param EndDate End date for statistics
	 * @return Task returning usage statistics
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate) = 0;

	// ========================================================================
	// LEGACY REPOSITORY METHODS - DEPRECATED
	// ========================================================================
	
	/**
	 * DEPRECATED: Load skill domain data by player ID
	 * Use 3-Layer Mapping Architecture: LoadUserSkillSlots() instead
	 * @param PlayerId The player ID to load skills for
	 * @return Task that returns skill domain data
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: LoadUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_LoadSkillsByPlayerId(const FGuid& PlayerId) = 0;

	/**
	 * DEPRECATED: Save skill domain data
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 * @param SkillData The skill domain data to save
	 * @return Task that returns success/failure
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_SaveSkillData(const FSkillDomain& SkillData) = 0;

	/**
	 * DEPRECATED: Register a skill to player's skill slots
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 * @param PlayerId Player ID
	 * @param SkillSlot Skill slot to register
	 * @return Task that returns updated skill data
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_RegisterSkillByPlayerId(const FGuid& PlayerId, const FSkillSlotDTO& SkillSlot) = 0;

	/**
	 * DEPRECATED: Unregister a skill from player's skill slots
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 * @param PlayerId Player ID
	 * @param SlotIndex Slot index to unregister
	 * @return Task that returns updated skill data
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_UnregisterSkillByPlayerId(const FGuid& PlayerId, int32 SlotIndex) = 0;

	/**
	 * DEPRECATED: Update skill cooldown state
	 * Use 3-Layer Mapping Architecture: UpdateSkillSlotCooldown() instead
	 * @param PlayerId Player ID
	 * @param SlotIndex Slot index
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that returns success/failure
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: UpdateSkillSlotCooldown() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_UpdateSkillCooldown(const FGuid& PlayerId, int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown) = 0;

};

/**
 * Legacy interface - DEPRECATED: Use ISkillRepositoryInterface instead
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USkillStateRepositoryInterface : public UBaseRepositoryInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillStateRepositoryInterface : public IBaseRepositoryInterface
{
	GENERATED_BODY()

public:
	virtual bool LoadSkillState(int32 PlayerInformation, USkillComponent& SkillComponentToPopulate, TArray<int32> fetchedSkillList) = 0;
	virtual bool LoadSkillStateFromDB_Temp(int32 PlayerInformation, const TArray<int32>& fetchedSkillList) = 0;
	virtual bool SaveSkillState(int32 PlayerInformation, const USkillComponent* SkillComponentToSave, TArray<int32> SkillPayloadList) = 0;
};