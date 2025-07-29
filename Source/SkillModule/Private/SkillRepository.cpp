
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
// 3-LAYER MAPPING ARCHITECTURE METHODS (RECOMMENDED)
// ============================================================================

UE::Tasks::TTask<FSkillRepositoryResult3Layer> USkillRepository::LoadUserSkillSlots(const FString& UserId, const FString& SlotKey)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, SlotKey]() -> FSkillRepositoryResult3Layer
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("DatabaseManager not available"));
		}

		if (UserId.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Invalid UserId"));
		}

		auto LoadTask = DBManager->LoadUserSkillSlots(UserId, SlotKey);
		TArray<FSkillSlotDatabaseDTO> LoadedSlots = LoadTask.GetResult();
		return FSkillRepositoryResult3Layer::SuccessWithSlots(LoadedSlots);
	});
}

UE::Tasks::TTask<FSkillRepositoryResult3Layer> USkillRepository::SaveUserSkillSlots(const FString& UserId, const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, SkillSlotDTOs]() -> FSkillRepositoryResult3Layer
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("DatabaseManager not available"));
		}

		if (UserId.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Invalid UserId"));
		}

		if (SkillSlotDTOs.Num() == 0)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("No skill slots to save"));
		}

		// Execute database operation on worker thread
		auto SaveTask = DBManager->SaveUserSkillSlots(SkillSlotDTOs);
		bool bSuccess = SaveTask.GetResult();

		if (bSuccess)
		{
			return FSkillRepositoryResult3Layer::SuccessWithSlots(SkillSlotDTOs);
		}
		else
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Failed to save skill slots to database"));
		}
	});
}

UE::Tasks::TTask<FSkillRepositoryResult3Layer> USkillRepository::LoadSkillMasterData(const TArray<int32>& SkillIds)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, SkillIds]() -> FSkillRepositoryResult3Layer
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("DatabaseManager not available"));
		}

		// Execute database operation on worker thread
		auto LoadTask = DBManager->LoadSkillMasterData(SkillIds);
		TArray<FSkillMasterDatabaseDTO> LoadedMasterData = LoadTask.GetResult();

		// Return the master data
		return FSkillRepositoryResult3Layer::SuccessWithMasterData(LoadedMasterData);
	});
}

UE::Tasks::TTask<FSkillRepositoryResult3Layer> USkillRepository::UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, SlotKey, SlotIndex, LastUsedTime]() -> FSkillRepositoryResult3Layer
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("DatabaseManager not available"));
		}

		if (UserId.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Invalid UserId"));
		}

		if (SlotKey.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("SlotKey cannot be empty"));
		}

		if (SlotIndex < 0)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Invalid SlotIndex"));
		}

		// Execute database operation on worker thread
		auto UpdateTask = DBManager->UpdateSkillSlotCooldown(UserId, SlotKey, SlotIndex, LastUsedTime);
		bool bSuccess = UpdateTask.GetResult();

		if (bSuccess)
		{
			// Reload updated slots
			auto ReloadTask = DBManager->LoadUserSkillSlots(UserId, SlotKey);
			TArray<FSkillSlotDatabaseDTO> UpdatedSlots = ReloadTask.GetResult();
			return FSkillRepositoryResult3Layer::SuccessWithSlots(UpdatedSlots);
		}
		else
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Failed to update skill cooldown"));
		}
	});
}

UE::Tasks::TTask<FSkillRepositoryResult3Layer> USkillRepository::ClearUserSkillSlots(const FString& UserId, const FString& SlotKey)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, SlotKey]() -> FSkillRepositoryResult3Layer
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("DatabaseManager not available"));
		}

		if (UserId.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Invalid UserId"));
		}

		if (SlotKey.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("SlotKey cannot be empty"));
		}

		// Execute database operation on worker thread
		auto ClearTask = DBManager->ClearUserSkillSlots(UserId, SlotKey);
		bool bSuccess = ClearTask.GetResult();

		if (bSuccess)
		{
			return FSkillRepositoryResult3Layer::SuccessWithSlots(TArray<FSkillSlotDatabaseDTO>());
		}
		else
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Failed to clear skill slots"));
		}
	});
}

UE::Tasks::TTask<FSkillRepositoryResult3Layer> USkillRepository::GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, SkillId, StartDate, EndDate]() -> FSkillRepositoryResult3Layer
	{
		if (!DBManager)
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("DatabaseManager not available"));
		}

		if (UserId.IsEmpty())
		{
			return FSkillRepositoryResult3Layer::Failure(TEXT("Invalid UserId"));
		}

		// Execute database operation on worker thread
		auto StatsTask = DBManager->GetSkillUsageStatistics(UserId, SkillId, StartDate, EndDate);
		TMap<int32, int32> UsageStats = StatsTask.GetResult();

		// For now, return success - you can extend this to include stats in the result
		return FSkillRepositoryResult3Layer::Success(FSkillDomain3Layer());
	});
}

// ============================================================================
// LEGACY REPOSITORY METHODS - DEPRECATED
// ============================================================================
//
// UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::DEP_LoadSkillsByPlayerId(const FGuid& PlayerId)
// {
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId]() -> FSkillRepositoryResult
// 	{
// 		if (!DBManager)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
// 		}
//
//
//
// 		// Convert FGuid PlayerId to FString UserId
// 		FString UserId = PlayerId.ToString();
//
// 		// Execute database operation on worker thread
// 		auto LoadTask = DBManager->LoadSkillsForPlayer(UserId);
// 		TArray<FSkillSlotDTO> LoadedSkills = LoadTask.GetResult();
//
// 		//?�재 DTO ?�이?��? ??가?�오지 못하?�것 같음.
// 		// Create domain object
// 		FSkillDomain SkillData(PlayerId, LoadedSkills);
// 		return FSkillRepositoryResult::Success(SkillData);
// 	});
// }
//
// UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::DEP_SaveSkillData(const FSkillDomain& SkillData)
// {
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, SkillData]() -> FSkillRepositoryResult
// 	{
// 		if (!DBManager)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
// 		}
//
// 		if (!SkillData.IsValid())
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Invalid skill data"));
// 		}
//
// 		// Convert FGuid PlayerId to FString UserId
// 		FString UserId = SkillData.PlayerId.ToString();
//
// 		// Execute database operation on worker thread
// 		auto SaveTask = DBManager->SaveSkillsForPlayer(UserId, SkillData.SkillSlots);
// 		bool bSuccess = SaveTask.GetResult();
//
// 		if (bSuccess)
// 		{
// 			return FSkillRepositoryResult::Success(SkillData);
// 		}
// 		else
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Failed to save skills to database"));
// 		}
// 	});
// }
//
// UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::DEP_RegisterSkillByPlayerId(const FGuid& PlayerId, const FSkillSlotDTO& SkillSlot)
// {
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SkillSlot]() -> FSkillRepositoryResult
// 	{
// 		if (!DBManager)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
// 		}
//
//
//
// 		// Convert FGuid PlayerId to FString UserId
// 		FString UserId = PlayerId.ToString();
//
// 		// Execute database operation on worker thread
// 		auto RegisterTask = DBManager->RegisterSkill(UserId, SkillSlot);
// 		bool bSuccess = RegisterTask.GetResult();
//
// 		if (bSuccess)
// 		{
// 			// Return updated skill data
// 			auto ReloadTask = DEP_LoadSkillsByPlayerId(PlayerId);
// 			return ReloadTask.GetResult();
// 		}
// 		else
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Failed to register skill to database"));
// 		}
// 	});
// }
//
// UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::DEP_UnregisterSkillByPlayerId(const FGuid& PlayerId, int32 SlotIndex)
// {
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SlotIndex]() -> FSkillRepositoryResult
// 	{
// 		if (!DBManager)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
// 		}
//
// 		if (SlotIndex < 0)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Invalid SlotIndex"));
// 		}
//
// 		// Convert FGuid PlayerId to FString UserId
// 		FString UserId = PlayerId.ToString();
//
// 		// Execute database operation on worker thread
// 		auto UnregisterTask = DBManager->UnregisterSkill(UserId, SlotIndex);
// 		bool bSuccess = UnregisterTask.GetResult();
//
// 		if (bSuccess)
// 		{
// 			// Return updated skill data
// 			auto ReloadTask = DEP_LoadSkillsByPlayerId(PlayerId);
// 			return ReloadTask.GetResult();
// 		}
// 		else
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Failed to unregister skill from database"));
// 		}
// 	});
// }
//
// UE::Tasks::TTask<FSkillRepositoryResult> USkillRepository::DEP_UpdateSkillCooldown(const FGuid& PlayerId, int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown)
// {
// 	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SlotIndex, LastUsedTime, RemainingCooldown]() -> FSkillRepositoryResult
// 	{
// 		if (!DBManager)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("DatabaseManager not available"));
// 		}
//
// 		if (SlotIndex < 0)
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Invalid SlotIndex"));
// 		}
//
// 		// Convert FGuid PlayerId to FString UserId
// 		FString UserId = PlayerId.ToString();
//
// 		// Execute database operation on worker thread
// 		auto UpdateTask = DBManager->UpdateSkillCooldown(UserId, SlotIndex, LastUsedTime, RemainingCooldown);
// 		bool bSuccess = UpdateTask.GetResult();
//
// 		if (bSuccess)
// 		{
// 			// Return updated skill data
// 			auto ReloadTask = DEP_LoadSkillsByPlayerId(PlayerId);
// 			return ReloadTask.GetResult();
// 		}
// 		else
// 		{
// 			return FSkillRepositoryResult::Failure(TEXT("Failed to update skill cooldown"));
// 		}
// 	});
// }
