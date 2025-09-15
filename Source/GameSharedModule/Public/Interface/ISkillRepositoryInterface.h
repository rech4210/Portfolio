#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interface/BaseRepositoryInterface.h"
#include "Tasks/Task.h"
#include "ISkillRepositoryInterface.generated.h"

struct FSkillSlotDatabaseDTO;
struct FSkillRepositoryResult3Layer;
class USkillDataAsset;
class USkillComponent;
class APlayerState;

UINTERFACE(MinimalAPI, Blueprintable)
class USkillRepositoryInterface : public UBaseRepositoryInterface
{
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API ISkillRepositoryInterface : public IBaseRepositoryInterface
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

};
