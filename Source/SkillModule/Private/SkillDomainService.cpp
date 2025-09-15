
#include "SkillDomainService.h"
#include "GameSharedModule/Public/Interface/ISkillRepositoryInterface.h"
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
	// DEBUG: 도메인 서비스 생성자 호출
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SkillDomainService Constructor called"));
}

void USkillDomainService::Initialize(TScriptInterface<ISkillRepositoryInterface> Repository)
{
	// DEBUG: 도메인 서비스 초기화 시작
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SkillDomainService::Initialize - Start"));
	
	// If no repository is provided, get it from the subsystem
	if (Repository.GetInterface())
	{
		SkillRepository = Repository;
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] Repository provided via parameter"));
	}
	else
	{
		// Get repository from GameInstance subsystem
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] Attempting to get repository from subsystem"));
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (USkillSubsystem* SkillSubsystem = GameInstance->GetSubsystem<USkillSubsystem>())
				{
					SkillRepository = SkillSubsystem->GetSkillRepository();
					UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] Repository retrieved from subsystem successfully"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] Failed to get SkillSubsystem from GameInstance"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] Failed to get GameInstance from World"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] Failed to get World"));
		}
	}
	
	bool bRepositoryValid = SkillRepository.GetInterface() != nullptr;
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SkillDomainService: Initialized with repository from %s - Valid: %s"), 
		Repository.GetInterface() ? TEXT("parameter") : TEXT("subsystem"), 
		bRepositoryValid ? TEXT("true") : TEXT("false"));
}

// ============================================================================
// SKILL SYSTEM METHODS
// ============================================================================
// ============================================================================
// Database -> DTO -> Asset -> Domain Model -> Entity
// ============================================================================

void USkillDomainService::LoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity || UserId.IsEmpty() || !SkillRepository.GetInterface())
	{
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] LoadPlayerSkills - No SkillComponent found on PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Use repository interface directly (no casting needed)
	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] LoadPlayerSkills - Repository interface not available"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	auto LoadSlotsTask = Repository->LoadUserSkillSlots(UserId, FString()); // Empty SlotKey loads all slots
	auto LoadMasterTask = Repository->LoadSkillMasterData();

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, LoadSlotsTask, LoadMasterTask]() mutable -> void
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] LoadPlayerSkills - Background task execution started"));
		
		auto SlotsResult = LoadSlotsTask.GetResult();
		auto MasterResult = LoadMasterTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, SlotsResult, MasterResult]()
		{
			if (SlotsResult.bSuccess && MasterResult.bSuccess)
			{
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					TSet<int32> UniqueSkillIDs;
					for (const auto& SlotDTO : SlotsResult.SkillSlots)
					{
						if (SlotDTO.SkillId > 0) 
						{
							UniqueSkillIDs.Add(SlotDTO.SkillId);
							UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] Found SkillId: %d in slot"), SlotDTO.SkillId);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("[SKILL_DEBUG] Invalid SkillId: %d found in slot"), SlotDTO.SkillId);
						}
					}

					TArray<USkillDataAsset*> SkillDataAssets;
					for (int32 SkillID : UniqueSkillIDs)
					{
						FPrimaryAssetId PrimaryAssetId;
						if (ULocalDataBaseLoader::CheckPrimaryAssetId(SkillID, PrimaryAssetId))
						{
							USkillDataAsset* SkillDataAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(PrimaryAssetId);
							if (SkillDataAsset)
							{
								SkillDataAssets.Add(SkillDataAsset);
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] FAILED - Could not load SkillDataAsset for SkillID: %d"), SkillID);
							}
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] FAILED - Invalid PrimaryAssetId for SkillID: %d"), SkillID);
						}
					}
					SkillComponent->BuildSkillSlotsFromMappers(SlotsResult.SkillSlots, SkillDataAssets);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] SkillComponent is null during GameThread processing"));
				}
				
				UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] Broadcasting OnSkillLoadCompleted"));
				OnSkillLoadCompleted.Broadcast(PlayerIdentity->GetPlayerGuid());
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("LoadPlayerSkills"));
			}
			else
			{
				FString ErrorMsg = FString::Printf(TEXT("Failed to load skills: Slots=%s, Master=%s"), 
					SlotsResult.bSuccess ? TEXT("OK") : *SlotsResult.ErrorMessage,
					MasterResult.bSuccess ? TEXT("OK") : *MasterResult.ErrorMessage);
				UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] FAILED - %s"), *ErrorMsg);
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), ErrorMsg);
			}
		});
	});
}

void USkillDomainService::SavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	// DEBUG: 플레이어 스킬 저장 시작
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Start for UserId: %s"), *UserId);
	
	if (!PlayerIdentity || UserId.IsEmpty() || !SkillRepository.GetInterface())
	{
		FString ErrorReason = !PlayerIdentity ? TEXT("PlayerIdentity is null") : 
							  UserId.IsEmpty() ? TEXT("UserId is empty") : 
							  TEXT("SkillRepository interface is null");
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] SavePlayerSkills - Invalid parameters: %s"), *ErrorReason);
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), 
			FString::Printf(TEXT("Invalid parameters for 3-layer skill saving: %s"), *ErrorReason));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] SavePlayerSkills - Could not cast PlayerIdentity to PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] SavePlayerSkills - No SkillComponent found on PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Extracting DTOs from SkillComponent"));
	
	// Extract DTOs from SkillComponent
	TArray<FSkillSlotDatabaseDTO> SkillSlotDTOs = SkillComponent->ExtractDTOsFromSkillSlots(UserId);
	
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Extracted %d DTOs"), SkillSlotDTOs.Num());
	
	// Use repository interface directly (no casting needed)
	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] SavePlayerSkills - Repository interface not available"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Starting repository save operation"));
	
	// Save skill slots
	auto SaveTask = Repository->SaveUserSkillSlots(UserId, SkillSlotDTOs);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, SaveTask]() mutable -> void
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Background save task started"));
		auto Result = SaveTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, Result]()
		{
			UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Back to GameThread: %s"), 
				Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
			
			if (Result.bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] SavePlayerSkills - Broadcasting OnSkillSaveCompleted"));
				OnSkillSaveCompleted.Broadcast(PlayerIdentity->GetPlayerGuid());
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("SavePlayerSkills3Layer"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] SavePlayerSkills - FAILED: %s"), *Result.ErrorMessage);
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::UpdatePlayerSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData)
{
	// DEBUG: 스킬 슬롯 업데이트 시작
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Start for UserId: %s, SlotIndex: %d, SkillData: %s"), 
		*UserId, SlotIndex, SkillData ? *SkillData->GetName() : TEXT("NULL"));
	
	if (!PlayerIdentity || UserId.IsEmpty() || SlotIndex < 0 || !SkillRepository.GetInterface())
	{
		FString ErrorReason = !PlayerIdentity ? TEXT("PlayerIdentity is null") : 
							  UserId.IsEmpty() ? TEXT("UserId is empty") : 
							  SlotIndex < 0 ? TEXT("Invalid SlotIndex") :
							  TEXT("SkillRepository interface is null");
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Invalid parameters: %s"), *ErrorReason);
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), 
			FString::Printf(TEXT("Invalid parameters for skill slot update: %s"), *ErrorReason));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Could not cast PlayerIdentity to PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - No SkillComponent found on PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Domain validation through Component (Aggregate)
	if (SkillData)
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Validating skill registration"));
		if (!SkillComponent->CanRegisterSkill(SlotIndex, SkillData))
		{
			UE_LOG(LogTemp, Warning, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Cannot register skill - domain rules violation"));
			OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot register skill - domain rules violation"));
			return;
		}
	}
	else 
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Validating skill unregistration"));
		if (!SkillComponent->CanUnregisterSkill(SlotIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Cannot unregister skill - domain rules violation"));
			OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Cannot unregister skill - domain rules violation"));
			return;
		}
	}

	// Apply change to aggregate
	if (SkillData)
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Registering skill to slot %d"), SlotIndex);
		bool bRegistrationSuccess = SkillComponent->RegisterSkill(SlotIndex, SkillData);
		if (!bRegistrationSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Failed to register skill in aggregate"));
			OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Failed to register skill in aggregate"));
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Skill registration successful"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Unregistering skill from slot %d"), SlotIndex);
		SkillComponent->UnregisterSkill(SlotIndex);
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Skill unregistration successful"));
	}

	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdatePlayerSkillSlot - Persisting changes via SavePlayerSkills"));
	// Persist using skill system
	SavePlayerSkills(PlayerIdentity, UserId);
}

void USkillDomainService::UpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime)
{
	// DEBUG: 스킬 쿨다운 업데이트 시작
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Start for UserId: %s, SlotIndex: %d"), *UserId, SlotIndex);
	
	if (!PlayerIdentity || UserId.IsEmpty() || SlotIndex < 0 || !SkillRepository.GetInterface())
	{
		FString ErrorReason = !PlayerIdentity ? TEXT("PlayerIdentity is null") : 
							  UserId.IsEmpty() ? TEXT("UserId is empty") : 
							  SlotIndex < 0 ? TEXT("Invalid SlotIndex") :
							  TEXT("SkillRepository interface is null");
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Invalid parameters: %s"), *ErrorReason);
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), 
			FString::Printf(TEXT("Invalid parameters for cooldown update: %s"), *ErrorReason));
		return;
	}

	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Repository interface not available"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Starting repository cooldown update"));
	
	// Update cooldown directly by slot index
	// Note: This will need to be updated when Repository interface is modified to support slot-index-only cooldown updates
	auto UpdateTask = Repository->UpdateSkillSlotCooldown(UserId, FString("ActionBar"), SlotIndex, LastUsedTime);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, UpdateTask]() mutable -> void
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Background task started"));
		auto Result = UpdateTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, Result]()
		{
			UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Back to GameThread: %s"), 
				Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
			
			if (Result.bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - Broadcasting success"));
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("UpdateSkillCooldown3Layer"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] UpdateSkillCooldown - FAILED: %s"), *Result.ErrorMessage);
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
			}
		});
	});
}

void USkillDomainService::ClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	// DEBUG: 플레이어 스킬 클리어 시작
	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Start for UserId: %s"), *UserId);
	
	if (!PlayerIdentity || UserId.IsEmpty() || !SkillRepository.GetInterface())
	{
		FString ErrorReason = !PlayerIdentity ? TEXT("PlayerIdentity is null") : 
							  UserId.IsEmpty() ? TEXT("UserId is empty") : 
							  TEXT("SkillRepository interface is null");
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Invalid parameters: %s"), *ErrorReason);
		OnSkillOperationFailed.Broadcast(PlayerIdentity ? PlayerIdentity->GetPlayerGuid() : FGuid(), 
			FString::Printf(TEXT("Invalid parameters for skill clearing: %s"), *ErrorReason));
		return;
	}

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Could not cast PlayerIdentity to PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] ClearPlayerSkills - No SkillComponent found on PlayerState"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("No SkillComponent found on PlayerState"));
		return;
	}

	// Use repository interface directly (no casting needed)
	ISkillRepositoryInterface* Repository = SkillRepository.GetInterface();
	if (!Repository)
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Repository interface not available"));
		OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("Repository interface not available"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Starting repository clear operation"));
	
	// Clear all skills for user (no SlotKey filter)
	auto ClearTask = Repository->ClearUserSkillSlots(UserId, FString());
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerIdentity, PlayerState, ClearTask]() mutable -> void
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Background clear task started"));
		auto Result = ClearTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerIdentity, PlayerState, Result]()
		{
			UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Back to GameThread: %s"), 
				Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
			
			if (Result.bSuccess)
			{
				// Clear skills in component as well
				USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
				if (SkillComponent)
				{
					UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Clearing skills in component"));
					int32 ClearedCount = 0;
					
					// Clear all skill slots in component
					for (int32 i = 0; i < SkillComponent->GetMaxSlotCount(); ++i)
					{
						if (SkillComponent->CanUnregisterSkill(i))
						{
							SkillComponent->UnregisterSkill(i);
							ClearedCount++;
							UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Cleared slot %d"), i);
						}
					}
					
					UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Component clearing complete: %d slots cleared"), ClearedCount);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[SKILL_DEBUG] ClearPlayerSkills - SkillComponent is null during component clearing"));
				}
				
				UE_LOG(LogTemp, Log, TEXT("[SKILL_DEBUG] ClearPlayerSkills - Broadcasting success"));
				OnSkillOperationSucceeded.Broadcast(PlayerIdentity->GetPlayerGuid(), TEXT("ClearPlayerSkills3Layer"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[SKILL_DEBUG] ClearPlayerSkills - FAILED: %s"), *Result.ErrorMessage);
				OnSkillOperationFailed.Broadcast(PlayerIdentity->GetPlayerGuid(), Result.ErrorMessage);
			}
		});
	});
}
