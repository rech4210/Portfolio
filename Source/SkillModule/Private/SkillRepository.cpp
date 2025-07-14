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



		// Convert int32 PlayerId to FString UserId using helper
		FString UserId = UPlayerIdHelper::ConvertPlayerIdToUserId(PlayerId);

		// Execute database operation on worker thread
		auto LoadTask = DBManager->LoadSkillsForPlayer(UserId);
		TArray<FSkillSlotDTO> LoadedSkills = LoadTask.GetResult();

		//현재 DTO 데이터를 잘 가져오지 못하는것 같음.
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

		// Convert int32 PlayerId to FString UserId using helper
		FString UserId = UPlayerIdHelper::ConvertPlayerIdToUserId(SkillData.PlayerId);

		// Execute database operation on worker thread
		auto SaveTask = DBManager->SaveSkillsForPlayer(UserId, SkillData.SkillSlots);
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



		// Convert int32 PlayerId to FString UserId using helper
		FString UserId = UPlayerIdHelper::ConvertPlayerIdToUserId(PlayerId);

		// Execute database operation on worker thread
		auto RegisterTask = DBManager->RegisterSkill(UserId, SkillSlot);
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



		if (!SlotId.IsValid())
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid SlotId"));
		}

		// Convert int32 PlayerId to FString UserId using helper
		FString UserId = UPlayerIdHelper::ConvertPlayerIdToUserId(PlayerId);

		// Execute database operation on worker thread
		auto UnregisterTask = DBManager->UnregisterSkill(UserId, SlotId);
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



		if (!SlotId.IsValid())
		{
			return FSkillRepositoryResult::Failure(TEXT("Invalid SlotId"));
		}

		// Convert int32 PlayerId to FString UserId using helper
		FString UserId = UPlayerIdHelper::ConvertPlayerIdToUserId(PlayerId);

		// Execute database operation on worker thread
		auto UpdateTask = DBManager->UpdateSkillCooldown(UserId, SlotId, LastUsedTime, RemainingCooldown);
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
