// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "SkillRepository.generated.h"

class USkillComponent;
class UDatabaseManager;

/**
 * Repository for managing skill persistence with async database operations
 */
UCLASS()
class SKILLMODULE_API USkillRepository : public UObject, public ISkillRepositoryInterface 
{
	GENERATED_BODY()

public:
	// ISkillRepositoryInterface implementation
	virtual void Initialize() override;

	// ========================================================================
	// 3-LAYER MAPPING ARCHITECTURE METHODS (RECOMMENDED)
	// ========================================================================
	
	/**
	 * Load user skill slots using 3-layer mapping architecture
	 * @param UserId User ID to load skills for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @return Task returning skill slot DTOs and master data
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> LoadUserSkillSlots(const FString& UserId, const FString& SlotKey) override;

	/**
	 * Save user skill slots using 3-layer mapping architecture
	 * @param UserId User ID to save skills for
	 * @param SkillSlotDTOs Skill slot DTOs to save
	 * @return Task returning success/failure result
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> SaveUserSkillSlots(const FString& UserId, const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs) override;

	/**
	 * Load skill master data using 3-layer mapping architecture
	 * @param SkillIds Optional list of specific skill IDs to load
	 * @return Task returning skill master data
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> LoadSkillMasterData(const TArray<int32>& SkillIds = {}) override;

	/**
	 * Update skill slot cooldown using 3-layer mapping architecture
	 * @param UserId User ID to update
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @param SlotIndex Slot index to update
	 * @param LastUsedTime When the skill was last used
	 * @return Task returning updated skill slots
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime) override;

	/**
	 * Clear all skill slots for a user using 3-layer mapping architecture
	 * @param UserId User ID to clear slots for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @return Task returning success/failure result
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> ClearUserSkillSlots(const FString& UserId, const FString& SlotKey) override;

	/**
	 * Get skill usage statistics using 3-layer mapping architecture
	 * @param UserId User ID to get stats for
	 * @param SkillId Specific skill ID (optional)
	 * @param StartDate Start date for statistics
	 * @param EndDate End date for statistics
	 * @return Task returning usage statistics
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate) override;

	// ========================================================================
	// LEGACY REPOSITORY METHODS - DEPRECATED
	// ========================================================================
	
	/**
	 * DEPRECATED: Load skills by player ID using legacy DDD approach
	 * Use 3-Layer Mapping Architecture: LoadUserSkillSlots() instead
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: LoadUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_LoadSkillsByPlayerId(const FGuid& PlayerId) override;

	/**
	 * DEPRECATED: Save skill data using legacy DDD approach
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_SaveSkillData(const FSkillDomain& SkillData) override;

	/**
	 * DEPRECATED: Register skill using legacy DDD approach
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_RegisterSkillByPlayerId(const FGuid& PlayerId, const FSkillSlotDTO& SkillSlot) override;

	/**
	 * DEPRECATED: Unregister skill using legacy DDD approach
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_UnregisterSkillByPlayerId(const FGuid& PlayerId, int32 SlotIndex) override;

	/**
	 * DEPRECATED: Update skill cooldown using legacy DDD approach
	 * Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	virtual UE::Tasks::TTask<FSkillRepositoryResult> DEP_UpdateSkillCooldown(const FGuid& PlayerId, int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};
