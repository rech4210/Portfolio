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
	// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES (RECOMMENDED)
	// ========================================================================
	virtual UE::Tasks::TTask<FSkillRepositoryResult> LoadSkillsByPlayerId(int32 PlayerId) override;
	virtual UE::Tasks::TTask<FSkillRepositoryResult> SaveSkillData(const FSkillDomain& SkillData) override;
	virtual UE::Tasks::TTask<FSkillRepositoryResult> RegisterSkillByPlayerId(int32 PlayerId, const FSkillSlotDTO& SkillSlot) override;
	virtual UE::Tasks::TTask<FSkillRepositoryResult> UnregisterSkillByPlayerId(int32 PlayerId, const FGuid& SlotId) override;
	virtual UE::Tasks::TTask<FSkillRepositoryResult> UpdateSkillCooldown(int32 PlayerId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown) override;

	// Legacy support methods
	virtual void RequestLoadSkillsForPlayer(APlayerState* PlayerState) override;
	virtual void RequestSaveSkillsForPlayer(APlayerState* PlayerState) override;
	virtual UE::Tasks::TTask<bool> LoadSkillsForPlayer(APlayerState* PlayerState) override;
	virtual UE::Tasks::TTask<bool> SaveSkillsForPlayer(APlayerState* PlayerState) override;
	virtual UE::Tasks::TTask<bool> RegisterSkillForPlayer(APlayerState* PlayerState, const FSkillSlotDTO& SkillSlot) override;
	virtual UE::Tasks::TTask<bool> UnregisterSkillForPlayer(APlayerState* PlayerState, const FGuid& SlotId) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};
