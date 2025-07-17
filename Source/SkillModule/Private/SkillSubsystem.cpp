// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillSubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "SkillRepository.h"
#include "SkillDomainService.h"
#include "Components/SkillComponent.h"
#include "Data/SkillDataAsset.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"

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

void USkillSubsystem::RequestRegisterSkill(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillDataAsset* SkillData)
{
	// 1. Network & Authority Validation (App Layer responsibility)
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill registration"));
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
		*PlayerIdentity->GetPlayerGuid().ToString(), SkillData ? SkillData->SkillID : -1);

	// 3. Domain Service Call (Delegate business logic)
	DomainService->RegisterSkillToPlayer(PlayerIdentity, SkillData);
}

void USkillSubsystem::RequestUnregisterSkill(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotId)
{
	// 1. Network & Authority Validation
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: Invalid PlayerIdentity for skill unregistration"));
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
		*PlayerIdentity->GetPlayerGuid().ToString(), *SlotId.ToString());

	// 3. Domain Service Call
	DomainService->UnregisterSkillFromPlayer(PlayerIdentity, SlotId);
}

void USkillSubsystem::RequestSwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotIdA, const FGuid& SlotIdB)
{
	// 1. Network & Authority Validation
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill swap transaction - Player: %s, SlotA: %s, SlotB: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *SlotIdA.ToString(), *SlotIdB.ToString());

	// 3. Domain Service Call
	DomainService->SwapSkillSlots(PlayerIdentity, SlotIdA, SlotIdB);
}

void USkillSubsystem::RequestLoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity)
{
	// 1. Network & Authority Validation
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill load transaction - Player: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString());

	// 3. Domain Service Call
	DomainService->LoadSkills(PlayerIdentity);
}

void USkillSubsystem::RequestSavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FSkillDomain& SkillData)
{
	// 1. Authority Validation
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting skill save transaction - Player: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString());

	// 3. Domain Service Call
	DomainService->SaveSkills(PlayerIdentity, SkillData);
}

void USkillSubsystem::RequestUpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	// 1. Authority Validation
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

	// 2. Transaction Boundary & Logging
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Starting cooldown update transaction - Player: %s, SlotId: %s"), 
		*PlayerIdentity->GetPlayerGuid().ToString(), *SlotId.ToString());

	// 3. Domain Service Call
	DomainService->UpdateSkillCooldown(PlayerIdentity, SlotId, LastUsedTime, RemainingCooldown);
}
