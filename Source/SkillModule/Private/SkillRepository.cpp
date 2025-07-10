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
		// Note: You'll need to implement LoadSkillsForPlayer in DatabaseManager
		// For now, we'll create a mock implementation
		TArray<FSkillSlotDTO> LoadedSkills;
		
		// TODO: Replace with actual database call
		// auto LoadTask = DBManager->LoadSkillsForPlayer(PlayerId);
		// LoadedSkills = LoadTask.GetResult();

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
		// TODO: Replace with actual database call
		// auto SaveTask = DBManager->SaveSkillsForPlayer(SkillData.PlayerId, SkillData.SkillSlots);
		// bool bSuccess = SaveTask.GetResult();
		bool bSuccess = true; // Mock implementation

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

		// if (!SkillSlot.IsValid())
		// {
		// 	return FSkillRepositoryResult::Failure(TEXT("Invalid skill slot"));
		// }

		// Execute database operation on worker thread
		// TODO: Replace with actual database call
		// auto RegisterTask = DBManager->RegisterSkill(PlayerId, SkillSlot);
		// bool bSuccess = RegisterTask.GetResult();
		bool bSuccess = true; // Mock implementation

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
		// TODO: Replace with actual database call
		// auto UnregisterTask = DBManager->UnregisterSkill(PlayerId, SlotId);
		// bool bSuccess = UnregisterTask.GetResult();
		bool bSuccess = true; // Mock implementation

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
		// TODO: Replace with actual database call
		// auto UpdateTask = DBManager->UpdateSkillCooldown(PlayerId, SlotId, LastUsedTime, RemainingCooldown);
		// bool bSuccess = UpdateTask.GetResult();
		bool bSuccess = true; // Mock implementation

		if (bSuccess)
		{
			// Return updated skill data
			auto ReloadTask = LoadSkillsByPlayerId(PlayerId);
			return ReloadTask.GetResult();
		}
		else
		{
			return FSkillRepositoryResult::Failure(TEXT("Failed to update skill cooldown in database"));
		}
	});
}

// ============================================================================
// LEGACY SUPPORT METHODS - ENGINE OBJECT DEPENDENCIES
// ============================================================================

UE::Tasks::TTask<bool> USkillRepository::LoadSkillsForPlayer(APlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, WeakPlayerState]() -> bool
	{
		// Execute pure repository operation on worker thread
		auto RepositoryTask = LoadSkillsByPlayerId(PlayerId);
		FSkillRepositoryResult Result = RepositoryTask.GetResult();

		if (Result.bSuccess)
		{
			// Update UI components on game thread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, Result]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (USkillComponent* SkillComponent = WeakPlayerState->FindComponentByClass<USkillComponent>())
					{
						// Convert domain data back to engine objects
						TArray<USkillSlot*> SkillSlots;
						
						for (const FSkillSlotDTO& SlotDTO : Result.SkillData.SkillSlots)
						{
							USkillSlot* NewSlot = NewObject<USkillSlot>(SkillComponent);
							if (NewSlot)
							{
								NewSlot->SlotId = SlotDTO.SlotId;
								NewSlot->LastUsedTime = SlotDTO.LastUsedTime;

								// Load SkillData from AssetManager
								if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
								{
									FPrimaryAssetId AssetId(TEXT("Skill"), *FString::FromInt(SlotDTO.SkillID));
									if (USkillDataAsset* SkillData = AssetManager->GetPrimaryAssetObject<USkillDataAsset>(AssetId))
									{
										NewSlot->SkillData = SkillData;
										NewSlot->AbilityClass = SkillData->AbilityClass;
									}
								}
								
								SkillSlots.Add(NewSlot);
							}
						}

						// Apply loaded skills to component
						// Note: You'll need to implement a method to set skill slots in SkillComponent
						// SkillComponent->Server_SetSkillSlots(SkillSlots);
					}
				}
			});
		}

		return Result.bSuccess;
	});
}

UE::Tasks::TTask<bool> USkillRepository::SaveSkillsForPlayer(APlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;
	const int32 PlayerId = PlayerState->GetPlayerId();

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, WeakPlayerState, PlayerId]() -> bool
	{
		// Get skill data from component on game thread
		TArray<FSkillSlotDTO> SkillSlotDTOs;
		bool bDataRetrieved = false;

		// Use AsyncTask to safely access game thread objects
		FEvent* DataRetrievedEvent = FPlatformProcess::GetSynchEventFromPool();
		AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, &SkillSlotDTOs, &bDataRetrieved, DataRetrievedEvent]()
		{
			if (WeakPlayerState.IsValid())
			{
				if (USkillComponent* SkillComponent = WeakPlayerState->FindComponentByClass<USkillComponent>())
				{
					const TArray<USkillSlot*>& Slots = SkillComponent->GetAllSkillSlots();

					// Convert skill slots to DTOs
					for (int32 i = 0; i < Slots.Num(); ++i)
					{
						if (USkillSlot* Slot = Slots[i])
						{
							if (Slot->SkillData)
							{
								FSkillSlotDTO DTO;
								DTO.SlotId = Slot->SlotId;
								DTO.SkillID = Slot->SkillData->SkillID;
								DTO.SlotIndex = i;
								DTO.LastUsedTime = Slot->LastUsedTime;
								DTO.bIsActive = true;
								
								// Serialize additional skill data to JSON if needed
								TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
								JsonObject->SetNumberField(TEXT("CoolTime"), Slot->SkillData->CoolTime);
								JsonObject->SetNumberField(TEXT("CostAmount"), Slot->SkillData->CostAmount);
								
								FString JsonString;
								TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
								FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
								DTO.SkillData = JsonString;
								
								SkillSlotDTOs.Add(DTO);
							}
						}
					}
					bDataRetrieved = true;
				}
			}
			DataRetrievedEvent->Trigger();
		});

		// Wait for data retrieval to complete
		DataRetrievedEvent->Wait();
		FPlatformProcess::ReturnSynchEventToPool(DataRetrievedEvent);

		if (!bDataRetrieved)
		{
			return false;
		}

		// Create domain object and save via pure repository
		FSkillDomain SkillData(PlayerId, SkillSlotDTOs);
		auto SaveTask = SaveSkillData(SkillData);
		FSkillRepositoryResult Result = SaveTask.GetResult();

		return Result.bSuccess;
	});
}

UE::Tasks::TTask<bool> USkillRepository::RegisterSkillForPlayer(APlayerState* PlayerState, const FSkillSlotDTO& SkillSlot)
{
	if (!PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SkillSlot, WeakPlayerState]() -> bool
	{
		// Execute pure repository operation on worker thread
		auto RegisterTask = RegisterSkillByPlayerId(PlayerId, SkillSlot);
		FSkillRepositoryResult Result = RegisterTask.GetResult();

		if (Result.bSuccess)
		{
			// Update UI components on game thread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, Result]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (USkillComponent* SkillComponent = WeakPlayerState->FindComponentByClass<USkillComponent>())
					{
						// Trigger reload through domain service to update UI properly
						UE_LOG(LogTemp, Log, TEXT("Skill registered successfully, triggering skill reload"));
					}
				}
			});
		}

		return Result.bSuccess;
	});
}

UE::Tasks::TTask<bool> USkillRepository::UnregisterSkillForPlayer(APlayerState* PlayerState, const FGuid& SlotId)
{
	if (!PlayerState)
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, SlotId, WeakPlayerState]() -> bool
	{
		// Execute pure repository operation on worker thread
		auto UnregisterTask = UnregisterSkillByPlayerId(PlayerId, SlotId);
		FSkillRepositoryResult Result = UnregisterTask.GetResult();

		if (Result.bSuccess)
		{
			// Update UI components on game thread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, Result]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (USkillComponent* SkillComponent = WeakPlayerState->FindComponentByClass<USkillComponent>())
					{
						// Trigger reload through domain service to update UI properly
						UE_LOG(LogTemp, Log, TEXT("Skill unregistered successfully, triggering skill reload"));
					}
				}
			});
		}

		return Result.bSuccess;
	});
}

void USkillRepository::RequestLoadSkillsForPlayer(APlayerState* PlayerState)
{
	// Legacy method - use AsyncTask for non-blocking operation
	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, WeakPlayerState = TWeakObjectPtr<APlayerState>(PlayerState)]()
	{
		if (WeakPlayerState.IsValid())
		{
			auto LoadTask = LoadSkillsForPlayer(WeakPlayerState.Get());
			bool bSuccess = LoadTask.GetResult();
			
			// Log result on GameThread
			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, bSuccess]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (bSuccess)
					{
						UE_LOG(LogTemp, Log, TEXT("Successfully loaded skills for player %s"), 
							*WeakPlayerState->GetPlayerName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to load skills for player %s"), 
							*WeakPlayerState->GetPlayerName());
					}
				}
			});
		}
	});
}

void USkillRepository::RequestSaveSkillsForPlayer(APlayerState* PlayerState)
{
	// Legacy method - use AsyncTask for non-blocking operation
	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, WeakPlayerState = TWeakObjectPtr<APlayerState>(PlayerState)]()
	{
		if (WeakPlayerState.IsValid())
		{
			auto SaveTask = SaveSkillsForPlayer(WeakPlayerState.Get());
			bool bSuccess = SaveTask.GetResult();
			
			// Log result on GameThread
			AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, bSuccess]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (bSuccess)
					{
						UE_LOG(LogTemp, Log, TEXT("Successfully saved skills for player %s"), 
							*WeakPlayerState->GetPlayerName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to save skills for player %s"), 
							*WeakPlayerState->GetPlayerName());
					}
				}
			});
		}
	});
}
