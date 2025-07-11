// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillDomainService.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "SkillSubsystem.h"
#include "Data/SkillDataAsset.h"
#include "Tasks/Task.h"
#include "Async/Async.h"

USkillDomainService::USkillDomainService()
{
	// Constructor
}

void USkillDomainService::Initialize(TScriptInterface<ISkillRepositoryInterface> Repository)
{
	// If no repository is provided, get it from the subsystem
	if (Repository.GetInterface())
	{
		SkillRepository = Repository;
	}
	else
	{
		// Get repository from GameInstance subsystem
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (USkillSubsystem* SkillSubsystem = GameInstance->GetSubsystem<USkillSubsystem>())
				{
					SkillRepository = SkillSubsystem->GetSkillRepository();
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("SkillDomainService: Initialized with repository from %s"), 
		Repository.GetInterface() ? TEXT("parameter") : TEXT("subsystem"));
}

void USkillDomainService::RegisterSkillToPlayer(int32 PlayerId, USkillDataAsset* SkillData)
{
	if (PlayerId <= 0 || !SkillData || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Invalid parameters for skill registration"));
		return;
	}

	// Create skill slot DTO from skill data
	FSkillSlotDTO NewSkillSlot;
	NewSkillSlot.SlotId = FGuid::NewGuid();
	NewSkillSlot.SkillID = SkillData->SkillID;
	NewSkillSlot.LastUsedTime = FDateTime::Now();

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->RegisterSkillByPlayerId(PlayerId, NewSkillSlot);
	
	ExecuteWithEvents(RepositoryTask, PlayerId, TEXT("RegisterSkill"));
}

void USkillDomainService::UnregisterSkillFromPlayer(int32 PlayerId, const FGuid& SlotId)
{
	if (PlayerId <= 0 || !SlotId.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Invalid parameters for skill unregistration"));
		return;
	}

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->UnregisterSkillByPlayerId(PlayerId, SlotId);
	
	ExecuteWithEvents(RepositoryTask, PlayerId, TEXT("UnregisterSkill"));
}

void USkillDomainService::SwapSkillSlots(int32 PlayerId, const FGuid& SlotIdA, const FGuid& SlotIdB)
{
	if (PlayerId <= 0 || !SlotIdA.IsValid() || !SlotIdB.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Invalid parameters for skill slot swap"));
		return;
	}

	// Load current skills, perform swap logic, and save
	auto LoadTask = SkillRepository.GetInterface()->LoadSkillsByPlayerId(PlayerId);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SlotIdA, SlotIdB, LoadTask]()mutable -> void
	{
		FSkillRepositoryResult LoadResult = LoadTask.GetResult();
		
		if (!LoadResult.bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, [this, PlayerId]()
			{
				OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Failed to load skills for swap"));
			});
			return;
		}

		// Perform swap logic in domain data
		FSkillDomain SkillData = LoadResult.SkillData;
		bool bSwapSuccess = false;
		
		// Find and swap the slots
		for (int32 i = 0; i < SkillData.SkillSlots.Num(); ++i)
		{
			for (int32 j = i + 1; j < SkillData.SkillSlots.Num(); ++j)
			{
				if ((SkillData.SkillSlots[i].SlotId == SlotIdA && SkillData.SkillSlots[j].SlotId == SlotIdB) ||
					(SkillData.SkillSlots[i].SlotId == SlotIdB && SkillData.SkillSlots[j].SlotId == SlotIdA))
				{
					Swap(SkillData.SkillSlots[i], SkillData.SkillSlots[j]);
					bSwapSuccess = true;
					break;
				}
			}
			if (bSwapSuccess) break;
		}

		if (!bSwapSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, [this, PlayerId]()
			{
				OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Skill slots not found for swap"));
			});
			return;
		}

		// Save updated skill data
		auto SaveTask = SkillRepository.GetInterface()->SaveSkillData(SkillData);
		FSkillRepositoryResult SaveResult = SaveTask.GetResult();

		AsyncTask(ENamedThreads::GameThread, [this, PlayerId, SaveResult]()
		{
			if (SaveResult.bSuccess)
			{
				OnSkillOperationSucceeded.Broadcast(PlayerId, TEXT("SwapSkillSlots"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Failed to save swapped skills"));
			}
		});
	});
}

void USkillDomainService::UpdateSkillCooldown(int32 PlayerId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	if (PlayerId <= 0 || !SlotId.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Invalid parameters for cooldown update"));
		return;
	}

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->UpdateSkillCooldown(PlayerId, SlotId, LastUsedTime, RemainingCooldown);
	
	ExecuteWithEvents(RepositoryTask, PlayerId, TEXT("UpdateCooldown"));
}

void USkillDomainService::LoadSkills(int32 PlayerId)
{
	if (PlayerId <= 0 || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Invalid parameters for skill loading"));
		return;
	}

	// Execute repository operation
	auto RepositoryTask = SkillRepository.GetInterface()->LoadSkillsByPlayerId(PlayerId);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, RepositoryTask]()mutable -> void
	{
		FSkillRepositoryResult Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerId, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillLoadCompleted.Broadcast(PlayerId);
				OnSkillOperationSucceeded.Broadcast(PlayerId, TEXT("LoadSkills"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerId, Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::SaveSkills(int32 PlayerId, const FSkillDomain& SkillData)
{
	if (PlayerId <= 0 || !SkillData.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerId, TEXT("Invalid parameters for skill saving"));
		return;
	}

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->SaveSkillData(SkillData);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, RepositoryTask]()mutable -> void
	{
		FSkillRepositoryResult Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerId, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillSaveCompleted.Broadcast(PlayerId);
				OnSkillOperationSucceeded.Broadcast(PlayerId, TEXT("SaveSkills"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerId, Result.ErrorMessage);
			}
		});
	});
}

template<typename T>
void USkillDomainService::ExecuteWithEvents(UE::Tasks::TTask<T> RepositoryTask, int32 PlayerId, const FString& OperationName)
{
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, OperationName, RepositoryTask]() -> void
	{
		auto Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerId, OperationName, Result]()
		{
			if constexpr (std::is_same_v<T, FSkillRepositoryResult>)
			{
				if (Result.bSuccess)
				{
					OnSkillOperationSucceeded.Broadcast(PlayerId, OperationName);
					
					// Trigger specific events based on operation
					if (OperationName == TEXT("RegisterSkill") && Result.SkillData.SkillSlots.Num() > 0)
					{
						// Find the most recently added skill
						const FSkillSlotDTO* LatestSkill = nullptr;
						FDateTime LatestTime = FDateTime::MinValue();
						
						for (const FSkillSlotDTO& Slot : Result.SkillData.SkillSlots)
						{
							if (Slot.LastUsedTime > LatestTime)
							{
								LatestTime = Slot.LastUsedTime;
								LatestSkill = &Slot;
							}
						}
						
						if (LatestSkill)
						{
							OnSkillRegistered.Broadcast(PlayerId, *LatestSkill);
						}
					}
				}
				else
				{
					OnSkillOperationFailed.Broadcast(PlayerId, Result.ErrorMessage);
				}
			}
		});
	});
}
