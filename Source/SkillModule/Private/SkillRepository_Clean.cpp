// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillRepository.h"
#include "Components/SkillComponent.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "SkillDomain.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "GameFramework/PlayerState.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Engine/AssetManager.h"

void USkillRepository::Initialize() 
{
	DBManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if (!DBManager) 
	{
		UE_LOG(LogTemp, Error, TEXT("SkillRepository: DatabaseManager is not available!"));
	}
}

// ============================================================================
// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES
// ============================================================================

UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::LoadSkillsByPlayerId(int32 PlayerId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId]() -> FSkillRepositoryResult
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}

		// Execute database operation on worker thread
		auto LoadTask = DBManager->LoadSkillsForPlayer(PlayerId);
		TArray<FSkillSlotDTO> LoadedSkills = LoadTask.GetResult();

		// Create domain object
		FSkillDomain SkillData(PlayerId, LoadedSkills);
		return FSkillRepositoryResult::Success(SkillData);
	});
}

UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::SaveSkillData(const FSkillDomain& SkillData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, SkillData]() -> FSkillRepositoryResult
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (!SkillData.IsValid())
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid skill data"));
		}

		// Execute database operation on worker thread
		auto SaveTask = DBManager->SaveSkillsForPlayer(SkillData.PlayerId, SkillData.SkillSlots);
		bool bSuccess = SaveTask.GetResult();

		if (bSuccess)
		{
			return FSkillRepositoryResult::Success(SkillData);
		}
		else
		{
			return FSkillRepositoryResult::Failure(TEXT("Failed to save skills to database"));
		}
	});
}

UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::RegisterSkillByPlayerId(int32 PlayerId, const FSkillSlotDTO& SkillSlot)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SkillSlot]() -> FSkillRepositoryResult
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}

		// Execute database operation on worker thread
		auto RegisterTask = DBManager->RegisterSkill(PlayerId, SkillSlot);
		bool bSuccess = RegisterTask.GetResult();

		if (bSuccess)
		{
			// Return updated skill data
			auto ReloadTask = LoadSkillsByPlayerId(PlayerId);
			return ReloadTask.GetResult();
		}
		else
		{
			return FSkillRepositoryResult::Failure(TEXT("Failed to register skill to database"));
		}
	});
}

UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::UnregisterSkillByPlayerId(int32 PlayerId, const FGuid& SlotId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SlotId]() -> FSkillRepositoryResult
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}

		if (!SlotId.IsValid())
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid SlotId"));
		}

		// Execute database operation on worker thread
		auto UnregisterTask = DBManager->UnregisterSkill(PlayerId, SlotId);
		bool bSuccess = UnregisterTask.GetResult();

		if (bSuccess)
		{
			// Return updated skill data
			auto ReloadTask = LoadSkillsByPlayerId(PlayerId);
			return ReloadTask.GetResult();
		}
		else
		{
			return FSkillRepositoryResult::Failure(TEXT("Failed to unregister skill from database"));
		}
	});
}

UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::UpdateSkillCooldown(int32 PlayerId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SlotId, LastUsedTime, RemainingCooldown]() -> FSkillRepositoryResult
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
		}

		if (PlayerId <= 0)
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid PlayerId"));
		}

		if (!SlotId.IsValid())
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid SlotId"));
		}

		// Execute database operation on worker thread
		auto UpdateTask = DBManager->UpdateSkillCooldown(PlayerId, SlotId, LastUsedTime, RemainingCooldown);
		bool bSuccess = UpdateTask.GetResult();

		if (bSuccess)
		{
			// Return updated skill data
			auto ReloadTask = LoadSkillsByPlayerId(PlayerId);
			return ReloadTask.GetResult();
		}
		else
		{
			return FSkillRepositoryResult::Failure(TEXT("Failed to update skill cooldown"));
		}
	});
}
