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
	// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES (RECOMMENDED)
	// ========================================================================
	
	/**
	 * Load skill domain data by player ID
	 * @param PlayerId The player ID to load skills for
	 * @return Task that returns skill domain data
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult> LoadSkillsByPlayerId(int32 PlayerId) = 0;

	/**
	 * Save skill domain data
	 * @param SkillData The skill domain data to save
	 * @return Task that returns success/failure
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult> SaveSkillData(const FSkillDomain& SkillData) = 0;

	/**
	 * Register a skill to player's skill slots
	 * @param PlayerId Player ID
	 * @param SkillSlot Skill slot to register
	 * @return Task that returns updated skill data
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult> RegisterSkillByPlayerId(int32 PlayerId, const FSkillSlotDTO& SkillSlot) = 0;

	/**
	 * Unregister a skill from player's skill slots
	 * @param PlayerId Player ID
	 * @param SlotId Slot ID to unregister
	 * @return Task that returns updated skill data
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult> UnregisterSkillByPlayerId(int32 PlayerId, const FGuid& SlotId) = 0;

	/**
	 * Update skill cooldown state
	 * @param PlayerId Player ID
	 * @param SlotId Slot ID
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that returns success/failure
	 */
	virtual UE::Tasks::TTask<FSkillRepositoryResult> UpdateSkillCooldown(int32 PlayerId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown) = 0;

	// ========================================================================
	// LEGACY SUPPORT METHODS - ENGINE OBJECT DEPENDENCIES
	// ========================================================================
	
	// Legacy methods for backward compatibility
	virtual void RequestLoadSkillsForPlayer(APlayerState* PlayerState) = 0;
	virtual void RequestSaveSkillsForPlayer(APlayerState* PlayerState) = 0;
	
	/**
	 * Load skills for player with engine object dependencies
	 * @param PlayerState Target player
	 * @return Task that completes when loading finishes
	 */
	virtual UE::Tasks::TTask<bool> LoadSkillsForPlayer(APlayerState* PlayerState) = 0;

	/**
	 * Save skills for player with engine object dependencies
	 * @param PlayerState Target player
	 * @return Task that completes when saving finishes
	 */
	virtual UE::Tasks::TTask<bool> SaveSkillsForPlayer(APlayerState* PlayerState) = 0;

	/**
	 * Register skill for player with engine object dependencies
	 * @param PlayerState Target player
	 * @param SkillSlot Skill slot to register
	 * @return Task that completes when registration finishes
	 */
	virtual UE::Tasks::TTask<bool> RegisterSkillForPlayer(APlayerState* PlayerState, const FSkillSlotDTO& SkillSlot) = 0;

	/**
	 * Unregister skill for player with engine object dependencies
	 * @param PlayerState Target player
	 * @param SlotId Slot ID to unregister
	 * @return Task that completes when unregistration finishes
	 */
	virtual UE::Tasks::TTask<bool> UnregisterSkillForPlayer(APlayerState* PlayerState, const FGuid& SlotId) = 0;
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