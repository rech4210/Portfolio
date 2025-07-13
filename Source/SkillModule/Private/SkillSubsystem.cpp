// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillSubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Repositories/SkillConfigRepository.h"
#include "Repositories/SkillStateRepository.h"
#include "SkillRepository.h"
#include "SkillDomainService.h"
#include "Components/SkillComponent.h"
#include "Data/SkillDataAsset.h"
#include "GameFramework/PlayerState.h"

void USkillSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// DB를 우선 초기화 진행
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	
	// Create default repository implementation
	DefaultSkillRepository = NewObject<USkillRepository>(this, TEXT("DefaultSkillRepository"));
	DefaultSkillRepository->Initialize();
	
	// Set as default if no other repository is injected
	if (!SkillRepositoryInterface.GetInterface())
	{
		SkillRepositoryInterface = DefaultSkillRepository;
	}

	DomainService = NewObject<USkillDomainService>(this);
	DomainService->Initialize(SkillRepositoryInterface);
	
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Created new SkillDomainService"));
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Initialized - Repository management only"));
}

void USkillSubsystem::Deinitialize()
{
	DefaultSkillRepository = nullptr;
	SkillRepositoryInterface = nullptr;
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
// Use Case Orchestration - App Layer Responsibilities Only
// ============================================================================

void USkillSubsystem::RequestRegisterSkill(APlayerState* PlayerState, USkillDataAsset* SkillData)
{
	// 1. Network & Authority Validation (App Layer responsibility)
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerState for skill registration"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Skill registration requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging (App Layer responsibility)
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill registration transaction - Player: %s, Skill: %d"), 
		*PlayerState->GetPlayerName(), SkillData ? SkillData->SkillID : -1);

	// 3. Domain Service Call (Delegate business logic)
	DomainService->RegisterSkillToPlayer(PlayerState, SkillData);
}

void USkillSubsystem::RequestUnregisterSkill(APlayerState* PlayerState, const FGuid& SlotId)
{
	// 1. Network & Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerState for skill unregistration"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Skill unregistration requests should only be made from server"));
		return;
	}

	if (!DomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillSubsystem: DomainService not initialized"));
		return;
	}

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill unregistration transaction - Player: %s, SlotId: %s"), 
		*PlayerState->GetPlayerName(), *SlotId.ToString());

	// 3. Domain Service Call
	DomainService->UnregisterSkillFromPlayer(PlayerState, SlotId);
}

void USkillSubsystem::RequestSwapSkillSlots(APlayerState* PlayerState, const FGuid& SlotIdA, const FGuid& SlotIdB)
{
	// 1. Network & Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerState for skill swap"));
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill swap transaction - Player: %s, SlotA: %s, SlotB: %s"), 
		*PlayerState->GetPlayerName(), *SlotIdA.ToString(), *SlotIdB.ToString());

	// 3. Domain Service Call
	DomainService->SwapSkillSlots(PlayerState, SlotIdA, SlotIdB);
}

void USkillSubsystem::RequestLoadPlayerSkills(APlayerState* PlayerState)
{
	// 1. Network & Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerState for skill loading"));
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill load transaction - Player: %s"), 
		*PlayerState->GetPlayerName());

	// 3. Domain Service Call
	DomainService->LoadSkills(PlayerState);
}

void USkillSubsystem::RequestSavePlayerSkills(APlayerState* PlayerState, const FSkillDomain& SkillData)
{
	// 1. Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerState for skill saving"));
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill save transaction - Player: %s"), 
		*PlayerState->GetPlayerName());

	// 3. Domain Service Call
	DomainService->SaveSkills(PlayerState, SkillData);
}

void USkillSubsystem::RequestUpdateSkillCooldown(APlayerState* PlayerState, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	// 1. Authority Validation
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerState for cooldown update"));
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting cooldown update transaction - Player: %s, SlotId: %s"), 
		*PlayerState->GetPlayerName(), *SlotId.ToString());

	// 3. Domain Service Call
	DomainService->UpdateSkillCooldown(PlayerState, SlotId, LastUsedTime, RemainingCooldown);
}
