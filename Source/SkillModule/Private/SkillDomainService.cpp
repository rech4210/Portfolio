// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillDomainService.h"
#include "Components/SkillComponent.h"
#include "SkillRepository.h"
#include "SkillSubsystem.h"
#include "Data/SkillDataAsset.h"
#include "Entities/SkillSlot.h"
#include "GameFramework/PlayerState.h"
#include "DatabaseModule/Public/DatabaseManager.h"
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

UE::Tasks::TTask<bool> USkillDomainService::RegisterSkillToPlayer(APlayerState* PlayerState, USkillDataAsset* SkillData)
{
	if (!PlayerState || !SkillData || !SkillRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SkillComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;
	TWeakObjectPtr<USkillDataAsset> WeakSkillData = SkillData;

	// Subscribe to domain events
	SubscribeToDomainEvents(SkillComponent);

	// Validate business rules on GameThread first
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, WeakPlayerState, WeakSkillData]() -> bool
	{
		if (!WeakPlayerState.IsValid() || !WeakSkillData.IsValid())
		{
			return false;
		}

		// Check if player can register this skill (business validation)
		bool bCanRegister = false;
		FSkillSlotDTO NewSkillSlot;

		// Use AsyncTask to safely access game thread objects for validation
		FEvent* ValidationEvent = FPlatformProcess::GetSynchEventFromPool();
		AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, WeakSkillData, &bCanRegister, &NewSkillSlot, ValidationEvent]()
		{
			if (WeakPlayerState.IsValid() && WeakSkillData.IsValid())
			{
				if (USkillComponent* SkillComp = WeakPlayerState->FindComponentByClass<USkillComponent>())
				{
					// Check if player has available slot and doesn't already have this skill
					const TArray<USkillSlot*>& ExistingSlots = SkillComp->GetAllSkillSlots();
					
					bool bAlreadyHasSkill = false;
					int32 AvailableSlotIndex = -1;

					for (int32 i = 0; i < SkillComp->GetMaxSlotCount(); ++i)
					{
						bool bSlotOccupied = false;
						for (const USkillSlot* Slot : ExistingSlots)
						{
							if (Slot && Slot->SkillData)
							{
								// Check if skill already exists
								if (Slot->SkillData->SkillID == WeakSkillData->SkillID)
								{
									bAlreadyHasSkill = true;
									break;
								}
								
								// This slot is occupied (assuming slots are indexed)
								if (ExistingSlots.Find(const_cast<USkillSlot*>(Slot)) == i)
								{
									bSlotOccupied = true;
								}
							}
						}
						
						if (bAlreadyHasSkill)
						{
							break;
						}
						
						if (!bSlotOccupied && AvailableSlotIndex == -1)
						{
							AvailableSlotIndex = i;
						}
					}

					if (!bAlreadyHasSkill && AvailableSlotIndex != -1)
					{
						// Create new skill slot DTO
						NewSkillSlot.SlotId = FGuid::NewGuid();
						NewSkillSlot.SkillID = WeakSkillData->SkillID;
						NewSkillSlot.SlotIndex = AvailableSlotIndex;
						NewSkillSlot.LastUsedTime = FDateTime::Now();
						NewSkillSlot.RemainingCooldown = 0.0f;
						NewSkillSlot.bIsActive = true;

						bCanRegister = true;
					}
				}
			}
			ValidationEvent->Trigger();
		});

		// Wait for validation to complete
		ValidationEvent->Wait();
		FPlatformProcess::ReturnSynchEventToPool(ValidationEvent);

		if (!bCanRegister)
		{
			// Execute failure callback on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState]()
			{
				if (WeakPlayerState.IsValid())
				{
					OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), TEXT("Cannot register skill: no available slots or skill already exists"));
				}
			});
			return false;
		}

		// Execute repository operation on worker thread
		if (USkillRepository* ConcreteRepo = Cast<USkillRepository>(SkillRepository.GetObject()))
		{
			auto RegisterTask = ConcreteRepo->RegisterSkillByPlayerId(WeakPlayerState->GetPlayerId(), NewSkillSlot);
			FSkillRepositoryResult Result = RegisterTask.GetResult();

			// Execute callbacks on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState, WeakSkillData, NewSkillSlot, Result]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (Result.bSuccess)
					{
						// Update the aggregate (SkillComponent) optimistically
						if (USkillComponent* SkillComp = WeakPlayerState->FindComponentByClass<USkillComponent>())
						{
							if (WeakSkillData.IsValid())
							{
								SkillComp->RegisterSkill(WeakSkillData.Get());
							}
						}

						OnSkillRegistered.Broadcast(WeakPlayerState.Get(), NewSkillSlot);
						OnSkillOperationSucceeded.Broadcast(WeakPlayerState.Get(), TEXT("Register Skill"));
					}
					else
					{
						OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), Result.ErrorMessage);
					}
				}
			});

			return Result.bSuccess;
		}

		// Fallback for interface-only access
		AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState]()
		{
			if (WeakPlayerState.IsValid())
			{
				OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), TEXT("Repository implementation not available"));
			}
		});
		return false;
	});
}

UE::Tasks::TTask<bool> USkillDomainService::UnregisterSkillFromPlayer(APlayerState* PlayerState, const FGuid& SlotId)
{
	if (!PlayerState || !SlotId.IsValid() || !SkillRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SkillComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, WeakPlayerState, SlotId]() -> bool
	{
		if (!WeakPlayerState.IsValid())
		{
			return false;
		}

		// Validate business rules on GameThread first
		bool bCanUnregister = false;

		FEvent* ValidationEvent = FPlatformProcess::GetSynchEventFromPool();
		AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, SlotId, &bCanUnregister, ValidationEvent]()
		{
			if (WeakPlayerState.IsValid())
			{
				if (USkillComponent* SkillComp = WeakPlayerState->FindComponentByClass<USkillComponent>())
				{
					// Check if the slot exists
					USkillSlot* ExistingSlot = SkillComp->GetSkillSlotByGuid(SlotId);
					bCanUnregister = (ExistingSlot != nullptr);
				}
			}
			ValidationEvent->Trigger();
		});

		// Wait for validation to complete
		ValidationEvent->Wait();
		FPlatformProcess::ReturnSynchEventToPool(ValidationEvent);

		if (!bCanUnregister)
		{
			AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState]()
			{
				if (WeakPlayerState.IsValid())
				{
					OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), TEXT("Cannot unregister skill: slot not found"));
				}
			});
			return false;
		}

		// Execute repository operation on worker thread
		if (USkillRepository* ConcreteRepo = Cast<USkillRepository>(SkillRepository.GetObject()))
		{
			auto UnregisterTask = ConcreteRepo->UnregisterSkillByPlayerId(WeakPlayerState->GetPlayerId(), SlotId);
			FSkillRepositoryResult Result = UnregisterTask.GetResult();

			// Execute callbacks on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState, SlotId, Result]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (Result.bSuccess)
					{
						// Update the aggregate (SkillComponent) optimistically
						if (USkillComponent* SkillComp = WeakPlayerState->FindComponentByClass<USkillComponent>())
						{
							SkillComp->UnregisterSkill(SlotId);
						}

						OnSkillUnregistered.Broadcast(WeakPlayerState.Get(), SlotId);
						OnSkillOperationSucceeded.Broadcast(WeakPlayerState.Get(), TEXT("Unregister Skill"));
					}
					else
					{
						OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), Result.ErrorMessage);
					}
				}
			});

			return Result.bSuccess;
		}

		return false;
	});
}

UE::Tasks::TTask<bool> USkillDomainService::SwapSkillSlots(APlayerState* PlayerState, const FGuid& SlotIdA, const FGuid& SlotIdB)
{
	if (!PlayerState || !SlotIdA.IsValid() || !SlotIdB.IsValid() || !SkillRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SkillComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, WeakPlayerState, SlotIdA, SlotIdB]() -> bool
	{
		if (!WeakPlayerState.IsValid())
		{
			return false;
		}

		// Perform swap on GameThread and then save
		bool bSwapSucceeded = false;

		AsyncTask(ENamedThreads::GameThread, [WeakPlayerState, SlotIdA, SlotIdB, &bSwapSucceeded]()
		{
			if (WeakPlayerState.IsValid())
			{
				if (USkillComponent* SkillComp = WeakPlayerState->FindComponentByClass<USkillComponent>())
				{
					SkillComp->SwapSkills(SlotIdA, SlotIdB);
					bSwapSucceeded = true;
				}
			}
		});

		if (bSwapSucceeded)
		{
			// Save the updated state
			auto SaveTask = SaveSkills(WeakPlayerState.Get());
			bool bSaveSucceeded = SaveTask.GetResult();

			AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState, bSaveSucceeded]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (bSaveSucceeded)
					{
						OnSkillOperationSucceeded.Broadcast(WeakPlayerState.Get(), TEXT("Swap Skills"));
					}
					else
					{
						OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), TEXT("Failed to save after skill swap"));
					}
				}
			});

			return bSaveSucceeded;
		}

		return false;
	});
}

UE::Tasks::TTask<bool> USkillDomainService::UpdateSkillCooldown(APlayerState* PlayerState, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	if (!PlayerState || !SlotId.IsValid() || !SkillRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	TWeakObjectPtr<APlayerState> WeakPlayerState = PlayerState;

	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, WeakPlayerState, SlotId, LastUsedTime, RemainingCooldown]() -> bool
	{
		if (!WeakPlayerState.IsValid())
		{
			return false;
		}

		// Execute repository operation on worker thread
		if (USkillRepository* ConcreteRepo = Cast<USkillRepository>(SkillRepository.GetObject()))
		{
			auto UpdateTask = ConcreteRepo->UpdateSkillCooldown(WeakPlayerState->GetPlayerId(), SlotId, LastUsedTime, RemainingCooldown);
			FSkillRepositoryResult Result = UpdateTask.GetResult();

			// Execute callbacks on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, WeakPlayerState, Result]()
			{
				if (WeakPlayerState.IsValid())
				{
					if (Result.bSuccess)
					{
						OnSkillOperationSucceeded.Broadcast(WeakPlayerState.Get(), TEXT("Update Skill Cooldown"));
					}
					else
					{
						OnSkillOperationFailed.Broadcast(WeakPlayerState.Get(), Result.ErrorMessage);
					}
				}
			});

			return Result.bSuccess;
		}

		return false;
	});
}

UE::Tasks::TTask<bool> USkillDomainService::LoadSkills(APlayerState* PlayerState)
{
	if (!PlayerState || !SkillRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SkillComponent found on PlayerState"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// Subscribe to domain events
	SubscribeToDomainEvents(SkillComponent);

	// Load through repository
	if (USkillRepository* ConcreteRepo = Cast<USkillRepository>(SkillRepository.GetObject()))
	{
		auto LoadTask = ConcreteRepo->LoadSkillsForPlayer(PlayerState);
		
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, LoadTask]() mutable -> bool
		{
			bool bSuccess = LoadTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, bSuccess]()
			{
				if (bSuccess)
				{
					OnSkillLoadCompleted.Broadcast(PlayerState);
					OnSkillOperationSucceeded.Broadcast(PlayerState, TEXT("Load Skills"));
				}
				else
				{
					OnSkillOperationFailed.Broadcast(PlayerState, TEXT("Failed to load skills from database"));
				}
			});
			
			return bSuccess;
		});
	}

	// Fallback for interface-only access
	SkillRepository->RequestLoadSkillsForPlayer(PlayerState);
	return UE::Tasks::MakeCompletedTask<bool>(true);
}

UE::Tasks::TTask<bool> USkillDomainService::SaveSkills(APlayerState* PlayerState)
{
	if (!PlayerState || !SkillRepository.GetInterface())
	{
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	// Save through repository
	if (USkillRepository* ConcreteRepo = Cast<USkillRepository>(SkillRepository.GetObject()))
	{
		auto SaveTask = ConcreteRepo->SaveSkillsForPlayer(PlayerState);
		
		return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, SaveTask]() mutable -> bool
		{
			bool bSuccess = SaveTask.GetResult(); // Wait for completion and get result
			
			// Execute UI updates on GameThread using AsyncTask
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, bSuccess]()
			{
				if (bSuccess)
				{
					OnSkillSaveCompleted.Broadcast(PlayerState);
					OnSkillOperationSucceeded.Broadcast(PlayerState, TEXT("Save Skills"));
				}
				else
				{
					OnSkillOperationFailed.Broadcast(PlayerState, TEXT("Failed to save skills to database"));
				}
			});
			
			return bSuccess;
		});
	}

	// Fallback for interface-only access
	SkillRepository->RequestSaveSkillsForPlayer(PlayerState);
	return UE::Tasks::MakeCompletedTask<bool>(true);
}

void USkillDomainService::SubscribeToDomainEvents(USkillComponent* SkillComponent)
{
	if (!SkillComponent)
	{
		return;
	}

	// Subscribe to domain events from the aggregate
	SkillComponent->OnSkillStateChanged.AddUFunction(this, FName("OnDomainSkillsChanged"));
}

void USkillDomainService::UnsubscribeFromDomainEvents(USkillComponent* SkillComponent)
{
	if (!SkillComponent)
	{
		return;
	}

	// Unsubscribe from domain events
	SkillComponent->OnSkillStateChanged.RemoveAll(this);
}

void USkillDomainService::OnDomainSkillRegistered(USkillDataAsset* SkillData)
{
	// Handle domain event - skill was registered
	UE_LOG(LogTemp, Log, TEXT("Domain Event: Skill registered - %s"), SkillData ? *SkillData->GetName() : TEXT("Unknown"));
}

void USkillDomainService::OnDomainSkillUnregistered(const FGuid& SlotId)
{
	// Handle domain event - skill was unregistered
	UE_LOG(LogTemp, Log, TEXT("Domain Event: Skill unregistered - SlotId: %s"), *SlotId.ToString());
}

void USkillDomainService::OnDomainSkillsChanged()
{
	// Handle domain event - skills state changed
	UE_LOG(LogTemp, Log, TEXT("Domain Event: Skills state changed"));
}
