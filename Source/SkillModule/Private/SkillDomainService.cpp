// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillDomainService.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "SkillSubsystem.h"
#include "Data/SkillDataAsset.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"
#include "Utill/LocalDataBaseLoader.h"

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

// ============================================================================
// MODERN SKILL SYSTEM METHODS (RECOMMENDED)
// ============================================================================

void USkillDomainService::LoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity || UserId.IsEmpty() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill loading"));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Use repository interface directly (no casting needed)
	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	// Load all skill slots and master data (no SlotKey filter)
	auto LoadSlotsTask = Repository->LoadUserSkillSlots(UserId, FString()); // Empty SlotKey loads all slots
	auto LoadMasterTask = Repository->LoadSkillMasterData();

	// Execute both tasks asynchronously
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, LoadSlotsTask, LoadMasterTask]() mutable -> void
	{
		auto SlotsResult = LoadSlotsTask.GetResult();
		auto MasterResult = LoadMasterTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, SlotsResult, MasterResult]()
		{
			if (SlotsResult.bSuccess && MasterResult.bSuccess)
			{
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					// ============================================================================
					// 3-LAYER MAPPING: Database → DTO → Asset → Domain Model → Entity
					// ============================================================================
					
					TArray<USkillDataAsset*> SkillDataAssets;
					
					// Extract unique skill IDs from database results
					TSet<int32> UniqueSkillIDs;
					for (const auto& SlotDTO : SlotsResult.SkillSlots)
					{
						if (SlotDTO.SkillId > 0) // Valid skill ID
						{
							UniqueSkillIDs.Add(SlotDTO.SkillId);
						}
					}
					
					// Load SkillDataAssets using Asset Manager for each unique skill ID
					for (int32 SkillID : UniqueSkillIDs)
					{
						FPrimaryAssetId PrimaryAssetId;
						if (ULocalDataBaseLoader::CheckPrimaryAssetId(SkillID, PrimaryAssetId))
						{
							USkillDataAsset* SkillDataAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(PrimaryAssetId);
							if (SkillDataAsset)
							{
								SkillDataAssets.Add(SkillDataAsset);
								UE_LOG(LogTemp, Log, TEXT("[SKILL] Loaded SkillDataAsset for SkillID: %d, Name: %s"), 
									SkillID, *SkillDataAsset->GetName());
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("[SKILL] Failed to load SkillDataAsset for SkillID: %d"), SkillID);
							}
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("[SKILL] Invalid PrimaryAssetId for SkillID: %d"), SkillID);
						}
					}
					
					// Use SkillComponent's 3-layer mapping to build skill slots
					// This will handle: DTO → VO (SkillDataAsset) → DomainModel → Entity mapping
					SkillComponent->BuildSkillSlotsFromMappers(SlotsResult.SkillSlots, SkillDataAssets);
					
					UE_LOG(LogTemp, Log, TEXT("[SKILL] ✓ Built skill slots from %d database entries and %d assets"), 
						SlotsResult.SkillSlots.Num(), SkillDataAssets.Num());
				}
				
				OnSkillLoadCompleted.Broadcast(PlayerIdentity->GetPlayerGuid());
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("LoadPlayerSkills"));
			}
			else
			{
				FString ErrorMsg = FString::Printf(TEXT("Failed to load skills: Slots=%s, Master=%s"), 
					SlotsResult.bSuccess ? TEXT("OK") : *SlotsResult.ErrorMessage,
					MasterResult.bSuccess ? TEXT("OK") : *MasterResult.ErrorMessage);
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), ErrorMsg);
			}
		});
	});
}

void USkillDomainService::SavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity || UserId.IsEmpty() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for 3-layer skill saving"));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Extract DTOs from SkillComponent
	TArray<FSkillSlotDatabaseDTO> SkillSlotDTOs = SkillComponent->ExtractDTOsFromSkillSlots(UserId);
	
	// Use repository interface directly (no casting needed)
	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	// Save skill slots
	auto SaveTask = Repository->SaveUserSkillSlots(UserId, SkillSlotDTOs);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, SaveTask]() mutable -> void
	{
		auto Result = SaveTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillSaveCompleted.Broadcast(PlayerIdentity->GetPlayerGuid());
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("SavePlayerSkills3Layer"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::UpdatePlayerSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData)
{
	if (!PlayerIdentity || UserId.IsEmpty() || SlotIndex < 0 || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill slot update"));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (SkillData && !SkillComponent->CanRegisterSkill(SlotIndex, SkillData))
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot register skill - domain rules violation"));
		return;
	}
	else if (!SkillData && !SkillComponent->CanUnregisterSkill(SlotIndex))
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot unregister skill - domain rules violation"));
		return;
	}

	// Apply change to aggregate
	if (SkillData)
	{
		bool bRegistrationSuccess = SkillComponent->RegisterSkill(SlotIndex, SkillData);
		if (!bRegistrationSuccess)
		{
			OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Failed to register skill in aggregate"));
			return;
		}
	}
	else
	{
		SkillComponent->UnregisterSkill(SlotIndex);
	}

	// Persist using skill system
	SavePlayerSkills(PlayerIdentity, UserId);
}

void USkillDomainService::UpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime)
{
	if (!PlayerIdentity || UserId.IsEmpty() || SlotIndex < 0 || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for cooldown update"));
		return;
	}

	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	// Update cooldown directly by slot index
	// Note: This will need to be updated when Repository interface is modified to support slot-index-only cooldown updates
	auto UpdateTask = Repository->UpdateSkillSlotCooldown(UserId, FString("ActionBar"), SlotIndex, LastUsedTime);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, UpdateTask]() mutable -> void
	{
		auto Result = UpdateTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, Result]()
		{
			if (Result.bSuccess)
			{
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("UpdateSkillCooldown3Layer"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::ClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity || UserId.IsEmpty() || !SkillRepository.GetInterface())
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill clearing"));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Use repository interface directly (no casting needed)
	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	// Clear all skills for user (no SlotKey filter)
	auto ClearTask = Repository->ClearUserSkillSlots(UserId, FString());
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, ClearTask]() mutable -> void
	{
		auto Result = ClearTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, Result]()
		{
			if (Result.bSuccess)
			{
				// Clear skills in component as well
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					// Clear all skill slots in component
					for (int32 i = 0; i < SkillComponent->GetMaxSlotCount(); ++i)
					{
						if (SkillComponent->CanUnregisterSkill(i))
						{
							SkillComponent->UnregisterSkill(i);
						}
					}
				}
				
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("ClearPlayerSkills3Layer"));
			}
			else
			{
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
			}
		});
	});
}

// ============================================================================
// LEGACY DOMAIN SERVICE METHODS - DEPRECATED
// ============================================================================
//
// void USkillDomainService::RegisterSkillToPlayer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillDataAsset* SkillData)
// {
// 	if (!PlayerIdentity || !SkillData || !SkillRepository.GetInterface())
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill registration"));
// 		return;
// 	}
//
// 	/*TODO: 객체를 가져오는것이 작동하는 검증*/
// 	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
// 	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
// 	if (!PlayerState)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
// 		return;
// 	}
//
// 	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 	if (!SkillComponent)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
// 		return;
// 	}
//
// 	int32 AvailableSlotIndex = SkillComponent->GetAvailableSlotIndex();
// 	if (AvailableSlotIndex == -1 || !SkillComponent->CanRegisterSkill(AvailableSlotIndex, SkillData))
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot register skill - domain rules violation"));
// 		return;
// 	}
//
// 	FSkillSlotDTO NewSkillSlot;
// 	// NewSkillSlot.SlotId = FGuid::NewGuid();
// 	NewSkillSlot.SkillID = SkillData->SkillID;
// 	NewSkillSlot.LastUsedTime = FDateTime::Now();
//
// 	// Apply optimistic update to Aggregate first
// 	bool bRegistrationSuccess = SkillComponent->RegisterSkill(AvailableSlotIndex, SkillData);
// 	if (!bRegistrationSuccess)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Failed to register skill in aggregate"));
// 		return;
// 	}
//
// 	// Execute repository operation with atomic transaction
// 	auto RepositoryTask = SkillRepository.GetInterface()->DEP_RegisterSkillByPlayerId(PlayerIdentity->GetPlayerGuid(), NewSkillSlot);
// 	
// 	// Enhanced error handling with rollback
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, SkillData, RepositoryTask]() mutable -> void
// 	{
// 		auto Result = RepositoryTask.GetResult();
// 		
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, SkillData, Result]()
// 		{
// 			if (Result.bSuccess)
// 			{
// 				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("RegisterSkill"));
// 			}
// 			else
// 			{
// 				// Rollback optimistic update
// 				/*실패에 따른 롤백을 제공할것.*/
// 				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 				if (SkillComponent)
// 				{
// 					// Find and remove the optimistically added skill
// 					// This requires implementing a way to identify the specific slot
// 					// For now, we'll log the error
// 					UE_LOG(LogTemp, Warning, TEXT("SkillDomainService: Failed to persist skill registration, manual rollback required"));
// 				}
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
// 			}
// 		});
// 	});
// }
//
// void USkillDomainService::UnregisterSkillFromPlayer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 SlotIndex)
// {
// 	if (!PlayerIdentity || SlotIndex < 0 || !SkillRepository.GetInterface())
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill unregistration"));
// 		return;
// 	}
//
// 	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
// 	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
// 	if (!PlayerState)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
// 		return;
// 	}
//
// 	// Get SkillComponent for domain validation
// 	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 	if (!SkillComponent)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
// 		return;
// 	}
//
// 	// Domain validation through Component (Aggregate)
// 	if (!SkillComponent->CanUnregisterSkill(SlotIndex))
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot unregister skill - domain rules violation"));
// 		return;
// 	}
//
// 	// Apply optimistic update to Aggregate first
// 	SkillComponent->UnregisterSkill(SlotIndex);
//
// 	// Execute repository operation with atomic transaction
// 	auto RepositoryTask = SkillRepository.GetInterface()->DEP_UnregisterSkillByPlayerId(PlayerIdentity->GetPlayerGuid(), SlotIndex);
// 	
// 	// Enhanced error handling with rollback
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, SlotIndex, RepositoryTask]() mutable -> void
// 	{
// 		auto Result = RepositoryTask.GetResult();
// 		
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, SlotIndex, Result]()
// 		{
// 			if (Result.bSuccess)
// 			{
// 				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("UnregisterSkill"));
// 			}
// 			else
// 			{
// 				// Rollback would require re-adding the skill, which is complex
// 				// For now, we'll reload from the repository to ensure consistency
// 				UE_LOG(LogTemp, Warning, TEXT("SkillDomainService: Failed to persist skill unregistration, reload required"));
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
// 			}
// 		});
// 	});
// }
//
// void USkillDomainService::SwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 SlotIndexA, int32 SlotIndexB)
// {
// 	if (!PlayerIdentity || SlotIndexA < 0 || SlotIndexB < 0 || !SkillRepository.GetInterface())
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill slot swap"));
// 		return;
// 	}
//
// 	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
// 	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
// 	if (!PlayerState)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
// 		return;
// 	}
//
// 	// Get SkillComponent for domain validation
// 	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 	if (!SkillComponent)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
// 		return;
// 	}
//
// 	// Domain validation through Component (Aggregate)
// 	if (!SkillComponent->CanSwapSkills(SlotIndexA, SlotIndexB))
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot swap skills - domain rules violation"));
// 		return;
// 	}
//
// 	// Apply optimistic update to Aggregate first
// 	SkillComponent->SwapSkills(SlotIndexA, SlotIndexB);
//
// 	// Load current skills, perform swap logic, and save
// 	auto LoadTask = SkillRepository.GetInterface()->DEP_LoadSkillsByPlayerId(PlayerIdentity->GetPlayerGuid());
// 	
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, SlotIndexA, SlotIndexB, LoadTask]()mutable -> void
// 	{
// 		FSkillRepositoryResult LoadResult = LoadTask.GetResult();
// 		
// 		if (!LoadResult.bSuccess)
// 		{
// 			AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity]()
// 			{
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Failed to load skills for swap"));
// 			});
// 			return;
// 		}
//
// 		// Perform swap logic in domain data
// 		FSkillDomain SkillData = LoadResult.SkillData;
// 		bool bSwapSuccess = false;
// 		
// 		// Find and swap the slots
// 		for (int32 i = 0; i < SkillData.SkillSlots.Num(); ++i)
// 		{
// 			for (int32 j = i + 1; j < SkillData.SkillSlots.Num(); ++j)
// 			{
// 				if ((SkillData.SkillSlots[i].SlotIndex == SlotIndexA && SkillData.SkillSlots[j].SlotIndex == SlotIndexB) ||
// 					(SkillData.SkillSlots[i].SlotIndex == SlotIndexB && SkillData.SkillSlots[j].SlotIndex == SlotIndexA))
// 				{
// 					Swap(SkillData.SkillSlots[i], SkillData.SkillSlots[j]);
// 					bSwapSuccess = true;
// 					break;
// 				}
// 			}
// 			if (bSwapSuccess) break;
// 		}
//
// 		if (!bSwapSuccess)
// 		{
// 			AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity]()
// 			{
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Skill slots not found for swap"));
// 			});
// 			return;
// 		}
//
// 		// Save updated skill data
// 		auto SaveTask = SkillRepository.GetInterface()->DEP_SaveSkillData(SkillData);
// 		FSkillRepositoryResult SaveResult = SaveTask.GetResult();
//
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, SlotIndexA, SlotIndexB, SaveResult]()
// 		{
// 			if (SaveResult.bSuccess)
// 			{
// 				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("SwapSkillSlots"));
// 			}
// 			else
// 			{
// 				// Rollback optimistic swap
// 				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 				if (SkillComponent)
// 				{
// 					SkillComponent->SwapSkills(SlotIndexB, SlotIndexA); // Swap back
// 				}
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Failed to save swapped skills"));
// 			}
// 		});
// 	});
// }
//
// void USkillDomainService::UpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown)
// {
// 	if (!PlayerIdentity || SlotIndex < 0 || !SkillRepository.GetInterface())
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for cooldown update"));
// 		return;
// 	}
//
// 	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
// 	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
// 	if (!PlayerState)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
// 		return;
// 	}
//
// 	// Get SkillComponent for domain validation
// 	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 	if (!SkillComponent)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
// 		return;
// 	}
//
// 	// Domain validation through Component (Aggregate)
// 	if (!SkillComponent->CanUpdateCooldown(SlotIndex, LastUsedTime, RemainingCooldown))
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot update cooldown - domain rules violation"));
// 		return;
// 	}
//
// 	// Execute repository operation with atomic transaction
// 	auto RepositoryTask = SkillRepository.GetInterface()->DEP_UpdateSkillCooldown(PlayerIdentity->GetPlayerGuid(), SlotIndex, LastUsedTime, RemainingCooldown);
// 	
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, RepositoryTask]() mutable -> void
// 	{
// 		auto Result = RepositoryTask.GetResult();
// 		
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, Result]()
// 		{
// 			if (Result.bSuccess)
// 			{
// 				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("UpdateCooldown"));
// 			}
// 			else
// 			{
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
// 			}
// 		});
// 	});
// }
//
// void USkillDomainService::LoadSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity)
// {
// 	if (!PlayerIdentity || !SkillRepository.GetInterface())
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill loading"));
// 		return;
// 	}
//
// 	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
// 	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
// 	if (!PlayerState)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
// 		return;
// 	}
//
// 	// Get SkillComponent for domain integration
// 	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 	if (!SkillComponent)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
// 		return;
// 	}
//
// 	// Execute repository operation
// 	auto RepositoryTask = SkillRepository.GetInterface()->DEP_LoadSkillsByPlayerId(PlayerIdentity->GetPlayerGuid());
// 	
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, RepositoryTask]()mutable -> void
// 	{
// 		FSkillRepositoryResult Result = RepositoryTask.GetResult();
// 		
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, Result]()
// 		{
// 			if (Result.bSuccess)
// 			{
// 				// Sync data with SkillComponent (Aggregate)
// 				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 				if (SkillComponent)
// 				{
// 					SkillComponent->SyncWithDomain(Result.SkillData);
// 				}
// 				
// 				OnSkillLoadCompleted.Broadcast(PlayerIdentity->GetPlayerGuid());
// 				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("LoadSkills"));
// 			}
// 			else
// 			{
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
// 			}
// 		});
// 	});
// }
//
// void USkillDomainService::SaveSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FSkillDomain& SkillData)
// {
// 	if (!PlayerIdentity || !SkillData.IsValid() || !SkillRepository.GetInterface())
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), TEXT("Invalid parameters for skill saving"));
// 		return;
// 	}
//
// 	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
// 	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
// 	if (!PlayerState)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
// 		return;
// 	}
//
// 	// Get SkillComponent for domain validation
// 	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
// 	if (!SkillComponent)
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
// 		return;
// 	}
//
// 	// Domain validation through Component (Aggregate)
// 	if (!SkillComponent->CanSaveSkills(SkillData))
// 	{
// 		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot save skills - domain rules violation"));
// 		return;
// 	}
//
// 	// Update aggregate with new data before persistence
// 	SkillComponent->SyncWithDomain(SkillData);
//
// 	// Execute repository operation with atomic transaction
// 	auto RepositoryTask = SkillRepository.GetInterface()->DEP_SaveSkillData(SkillData);
// 	
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, RepositoryTask]()mutable -> void
// 	{
// 		FSkillRepositoryResult Result = RepositoryTask.GetResult();
// 		
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, Result]()
// 		{
// 			if (Result.bSuccess)
// 			{
// 				OnSkillSaveCompleted.Broadcast(PlayerIdentity->GetPlayerGuid());
// 				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("SaveSkills"));
// 			}
// 			else
// 			{
// 				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
// 			}
// 		});
// 	});
// }
//
// template<typename T>
// void USkillDomainService::ExecuteWithEvents(UE::Tasks::TTask<T> RepositoryTask, const FGuid& PlayerGuid, const FString& OperationName)
// {
// 	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerGuid, OperationName, RepositoryTask]() mutable -> void
// 	{
// 		auto Result = RepositoryTask.GetResult();
// 		
// 		AsyncTask(ENamedThreads::GameThread, [this, PlayerGuid, OperationName, Result]()
// 		{
// 			if constexpr (std::is_same_v<T, FSkillRepositoryResult>)
// 			{
// 				if (Result.bSuccess)
// 				{
// 					OnSkillOperationSucceeded.Broadcast(PlayerGuid, OperationName);
// 					
// 					// Trigger specific events based on operation
// 					if (OperationName == TEXT("RegisterSkill") && Result.SkillData.SkillSlots.Num() > 0)
// 					{
// 						// Find the most recently added skill
// 						const FSkillSlotDTO* LatestSkill = nullptr;
// 						FDateTime LatestTime = FDateTime::MinValue();
// 						
// 						for (const FSkillSlotDTO& Slot : Result.SkillData.SkillSlots)
// 						{
// 							if (Slot.LastUsedTime > LatestTime)
// 							{
// 								LatestTime = Slot.LastUsedTime;
// 								LatestSkill = &Slot;
// 							}
// 						}
// 						
// 						if (LatestSkill)
// 						{
// 							OnSkillDomainRegistered.Broadcast(PlayerGuid, *LatestSkill);
// 						}
// 					}
// 				}
// 				else
// 				{
// 					OnSkillOperationFailed.Broadcast(PlayerGuid, Result.ErrorMessage);
// 				}
// 			}
// 		});
// 	});
// }
