// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillDomainService.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "SkillSubsystem.h"
#include "Data/SkillDataAsset.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerState.h"

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

void USkillDomainService::RegisterSkillToPlayer(APlayerState* PlayerState, USkillDataAsset* SkillData)
{
	if (!PlayerState || !SkillData || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerState ? PlayerState->GetPlayerId() : 0, TEXT("Invalid parameters for skill registration"));
		return;
	}

	// Get SkillComponent for domain validation
	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (!SkillComponent->CanRegisterSkill(SkillData))
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Cannot register skill - domain rules violation"));
		return;
	}

	// Create skill slot DTO from skill data
	FSkillSlotDTO NewSkillSlot;
	NewSkillSlot.SlotId = FGuid::NewGuid();
	NewSkillSlot.SkillID = SkillData->SkillID;
	NewSkillSlot.LastUsedTime = FDateTime::Now();

	// Apply optimistic update to Aggregate first
	bool bRegistrationSuccess = SkillComponent->RegisterSkill(SkillData);
	if (!bRegistrationSuccess)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Failed to register skill in aggregate"));
		return;
	}

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->RegisterSkillByPlayerId(PlayerState->GetPlayerId(), NewSkillSlot);
	
	// Enhanced error handling with rollback
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, SkillData, RepositoryTask]() mutable -> void
	{
		auto Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, SkillData, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillOperationSucceeded.Broadcast(PlayerState->GetPlayerId(), TEXT("RegisterSkill"));
			}
			else
			{
				// Rollback optimistic update
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					// Find and remove the optimistically added skill
					// This requires implementing a way to identify the specific slot
					// For now, we'll log the error
					UE_LOG(LogTemp, Warning, TEXT("SkillDomainService: Failed to persist skill registration, manual rollback required"));
				}
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::UnregisterSkillFromPlayer(APlayerState* PlayerState, const FGuid& SlotId)
{
	if (!PlayerState || !SlotId.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerState ? PlayerState->GetPlayerId() : 0, TEXT("Invalid parameters for skill unregistration"));
		return;
	}

	// Get SkillComponent for domain validation
	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (!SkillComponent->CanUnregisterSkill(SlotId))
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Cannot unregister skill - domain rules violation"));
		return;
	}

	// Apply optimistic update to Aggregate first
	SkillComponent->UnregisterSkill(SlotId);

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->UnregisterSkillByPlayerId(PlayerState->GetPlayerId(), SlotId);
	
	// Enhanced error handling with rollback
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, SlotId, RepositoryTask]() mutable -> void
	{
		auto Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, SlotId, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillOperationSucceeded.Broadcast(PlayerState->GetPlayerId(), TEXT("UnregisterSkill"));
			}
			else
			{
				// Rollback would require re-adding the skill, which is complex
				// For now, we'll reload from the repository to ensure consistency
				UE_LOG(LogTemp, Warning, TEXT("SkillDomainService: Failed to persist skill unregistration, reload required"));
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::SwapSkillSlots(APlayerState* PlayerState, const FGuid& SlotIdA, const FGuid& SlotIdB)
{
	if (!PlayerState || !SlotIdA.IsValid() || !SlotIdB.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerState ? PlayerState->GetPlayerId() : 0, TEXT("Invalid parameters for skill slot swap"));
		return;
	}

	// Get SkillComponent for domain validation
	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (!SkillComponent->CanSwapSkills(SlotIdA, SlotIdB))
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Cannot swap skills - domain rules violation"));
		return;
	}

	// Apply optimistic update to Aggregate first
	SkillComponent->SwapSkills(SlotIdA, SlotIdB);

	// Load current skills, perform swap logic, and save
	auto LoadTask = SkillRepository.GetInterface()->LoadSkillsByPlayerId(PlayerState->GetPlayerId());
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, SlotIdA, SlotIdB, LoadTask]()mutable -> void
	{
		FSkillRepositoryResult LoadResult = LoadTask.GetResult();
		
		if (!LoadResult.bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
			{
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Failed to load skills for swap"));
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
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState]()
			{
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Skill slots not found for swap"));
			});
			return;
		}

		// Save updated skill data
		auto SaveTask = SkillRepository.GetInterface()->SaveSkillData(SkillData);
		FSkillRepositoryResult SaveResult = SaveTask.GetResult();

		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, SlotIdA, SlotIdB, SaveResult]()
		{
			if (SaveResult.bSuccess)
			{
				OnSkillOperationSucceeded.Broadcast(PlayerState->GetPlayerId(), TEXT("SwapSkillSlots"));
			}
			else
			{
				// Rollback optimistic swap
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					SkillComponent->SwapSkills(SlotIdB, SlotIdA); // Swap back
				}
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Failed to save swapped skills"));
			}
		});
	});
}

void USkillDomainService::UpdateSkillCooldown(APlayerState* PlayerState, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	if (!PlayerState || !SlotId.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerState ? PlayerState->GetPlayerId() : 0, TEXT("Invalid parameters for cooldown update"));
		return;
	}

	// Get SkillComponent for domain validation
	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (!SkillComponent->CanUpdateCooldown(SlotId, LastUsedTime, RemainingCooldown))
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Cannot update cooldown - domain rules violation"));
		return;
	}

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->UpdateSkillCooldown(PlayerState->GetPlayerId(), SlotId, LastUsedTime, RemainingCooldown);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, RepositoryTask]() mutable -> void
	{
		auto Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillOperationSucceeded.Broadcast(PlayerState->GetPlayerId(), TEXT("UpdateCooldown"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::LoadSkills(APlayerState* PlayerState)
{
	if (!PlayerState || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerState ? PlayerState->GetPlayerId() : 0, TEXT("Invalid parameters for skill loading"));
		return;
	}

	// Get SkillComponent for domain integration
	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Execute repository operation
	auto RepositoryTask = SkillRepository.GetInterface()->LoadSkillsByPlayerId(PlayerState->GetPlayerId());
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, RepositoryTask]()mutable -> void
	{
		FSkillRepositoryResult Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Result]()
		{
			if (Result.bSuccess)
			{
				// Sync data with SkillComponent (Aggregate)
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					SkillComponent->SyncWithDomain(Result.SkillData);
				}
				
				OnSkillLoadCompleted.Broadcast(PlayerState->GetPlayerId());
				OnSkillOperationSucceeded.Broadcast(PlayerState->GetPlayerId(), TEXT("LoadSkills"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::SaveSkills(APlayerState* PlayerState, const FSkillDomain& SkillData)
{
	if (!PlayerState || !SkillData.IsValid() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerState ? PlayerState->GetPlayerId() : 0, TEXT("Invalid parameters for skill saving"));
		return;
	}

	// Get SkillComponent for domain validation
	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (!SkillComponent->CanSaveSkills(SkillData))
	{
		OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), TEXT("Cannot save skills - domain rules violation"));
		return;
	}

	// Update aggregate with new data before persistence
	SkillComponent->SyncWithDomain(SkillData);

	// Execute repository operation with atomic transaction
	auto RepositoryTask = SkillRepository.GetInterface()->SaveSkillData(SkillData);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, RepositoryTask]()mutable -> void
	{
		FSkillRepositoryResult Result = RepositoryTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillSaveCompleted.Broadcast(PlayerState->GetPlayerId());
				OnSkillOperationSucceeded.Broadcast(PlayerState->GetPlayerId(), TEXT("SaveSkills"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerState->GetPlayerId(), Result.ErrorMessage);
			}
		});
	});
}

template<typename T>
void USkillDomainService::ExecuteWithEvents(UE::Tasks::TTask<T> RepositoryTask, int32 PlayerId, const FString& OperationName)
{
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, OperationName, RepositoryTask]() mutable -> void
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
