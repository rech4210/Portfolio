#include "SkillSubsystem.h"
#if WITH_SERVER_CODE
#include "DatabaseModule/Public/Provider/DBProviderInfra.h"
#endif
#include "SkillRepository.h"
#include "SkillDomainService.h"
#include "Components/SkillComponent.h"
#include "Data/SkillDataAsset.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"

void USkillSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	
	DefaultSkillRepository = NewObject<USkillRepository>(this, TEXT("DefaultSkillRepository"));
#if WITH_SERVER_CODE
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto ProviderInfra = GI->GetSubsystem<UDBProviderInfra>())
		{
			DefaultSkillRepository->Initialize(ProviderInfra);
		}
	}
#endif
	
	if (!SkillRepositoryInterface.GetInterface())
	{
		SkillRepositoryInterface = DefaultSkillRepository;
	}

	DomainService = NewObject<USkillDomainService>(this);
	DomainService->Initialize(SkillRepositoryInterface);
	
	DomainService->OnSkillLoadCompleted.AddUObject(this, &USkillSubsystem::OnPlayerSkillsLoaded);
	
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Created new SkillDomainService"));
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Initialized - Repository management only"));
}

void USkillSubsystem::Deinitialize()
{
	if (DomainService)
	{
		DomainService->OnSkillLoadCompleted.RemoveAll(this);
	}
	
	DefaultSkillRepository = nullptr;
	SkillRepositoryInterface = nullptr;
	DomainService = nullptr;
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Deinitialized"));
}

TScriptInterface<ISkillRepositoryInterface> USkillSubsystem::GetSkillRepository() const
{
	return SkillRepositoryInterface;
}

void USkillSubsystem::SetSkillRepository(TScriptInterface<ISkillRepositoryInterface> Repository)
{
	SkillRepositoryInterface = Repository;
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Repository implementation changed"));
}

USkillDomainService* USkillSubsystem::GetDomainService() {
	return DomainService;	
}

// ============================================================================
// MODERN SKILL SYSTEM USE CASES (RECOMMENDED)
// ============================================================================

void USkillSubsystem::RequestLoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill loading"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Client should wait for replicated data, not load from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill load transaction - Player: %s, UserId: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *UserId);

	DomainService->LoadPlayerSkills(PlayerIdentity, UserId);
}

void USkillSubsystem::RequestSavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill saving"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Skill save requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill save transaction - Player: %s, UserId: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *UserId);

	DomainService->SavePlayerSkills(PlayerIdentity, UserId);
}

void USkillSubsystem::RequestUpdateSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData)
{
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill slot update"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Skill slot update requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill slot update transaction - Player: %s, UserId: %s, SlotIndex: %d, Skill: %d"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *UserId, SlotIndex, SkillData ? SkillData->SkillID : -1);

	DomainService->UpdatePlayerSkillSlot(PlayerIdentity, UserId, SlotIndex, SkillData);
}

void USkillSubsystem::RequestUpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime)
{
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for cooldown update"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Cooldown update requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting cooldown update transaction - Player: %s, UserId: %s, SlotIndex: %d"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *UserId, SlotIndex);

	DomainService->UpdateSkillCooldown(PlayerIdentity, UserId, SlotIndex, LastUsedTime);
}

void USkillSubsystem::RequestClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId)
{
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill clearing"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Skill clear requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill clear transaction - Player: %s, UserId: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *UserId);

	DomainService->ClearPlayerSkills(PlayerIdentity, UserId);
}

void USkillSubsystem::RequestSwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndexA, int32 SlotIndexB)
{
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill swap"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Skill swap requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill swap transaction - Player: %s, UserId: %s, SlotA: %d, SlotB: %d"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *UserId, SlotIndexA, SlotIndexB);

	UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
	APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Could not cast PlayerIdentity to PlayerState"));
		return;
	}

	USkillComponent* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>();
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: No SkillComponent found on PlayerState"));
		return;
	}

	if (SkillComponent->CanSwapSkills(SlotIndexA, SlotIndexB))
	{
		SkillComponent->SwapSkills(SlotIndexA, SlotIndexB);
		DomainService->SavePlayerSkills(PlayerIdentity, UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Cannot swap skills - domain rules violation"));
	}
}

void USkillSubsystem::OnPlayerSkillsLoaded(const FGuid& PlayerGuid)
{
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Skills loaded for player %s, triggering UI initialization"), *PlayerGuid.ToString());
	
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: World not available for UI initialization"));
		return;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (!PC || !PC->PlayerState)
			continue;

		if (auto PlayerIdentity = Cast<IPlayerIdentityInterface>(PC->PlayerState))
		{
			if (PlayerIdentity->GetPlayerGuid() == PlayerGuid)
			{
				UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Found matching player %s, searching for SkillComponent"), *PlayerGuid.ToString());
				
				if (USkillComponent* SkillComponent = PC->PlayerState->FindComponentByClass<USkillComponent>())
				{
					UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: SkillComponent found at %p"), SkillComponent);
					OnSkillDataLoadCompleted.Broadcast(TScriptInterface<IPlayerIdentityInterface>(PC->PlayerState), SkillComponent);
					UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: UI initialization event broadcasted for player %s"), *PlayerGuid.ToString());
					return;
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: SkillComponent not found on PlayerState for player %s"), *PlayerGuid.ToString());
					
					TArray<UActorComponent*> Components = PC->PlayerState->GetComponents().Array();
					UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: PlayerState has %d components:"), Components.Num());
					for (UActorComponent* Component : Components)
					{
						if (Component)
						{
							UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Component->GetClass()->GetName());
						}
					}
				}
				break;
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Player %s not found for UI initialization"), *PlayerGuid.ToString());
}
