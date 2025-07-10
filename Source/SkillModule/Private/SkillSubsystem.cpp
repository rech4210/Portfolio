// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillSubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Repositories/SkillConfigRepository.h"
#include "Repositories/SkillStateRepository.h"
#include "SkillRepository.h"
#include "SkillDomainService.h"
#include "Components/SkillComponent.h"
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

	// Legacy repositories
	SkillConfigRepository = NewObject<USkillConfigRepository>(this, TEXT("SkillConfigRepository"));
	SkillConfigRepository->Initialize();
	SkillStateRepository = NewObject<USkillStateRepository>(this, TEXT("SkillStateRepository"));
	SkillStateRepository->Initialize();

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Initialized - Repository management only"));
}

void USkillSubsystem::Deinitialize()
{
	DefaultSkillRepository = nullptr;
	SkillRepositoryInterface = nullptr;
	SkillConfigRepository = nullptr;
	SkillStateRepository = nullptr;
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

USkillDomainService* USkillSubsystem::CreateDomainService()
{
	USkillDomainService* DomainService = NewObject<USkillDomainService>(this);
	DomainService->Initialize(SkillRepositoryInterface);
	
	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Created new SkillDomainService"));
	return DomainService;
}

TScriptInterface<ISkillConfigRepositoryInterface> USkillSubsystem::GetSkillConfigRepository() const
{
	return SkillConfigRepository;
}

TScriptInterface<ISkillStateRepositoryInterface> USkillSubsystem::GetSkillStateRepository() const
{
	return SkillStateRepository;
}

void USkillSubsystem::RequestLoadSkillData(APlayerState* PlayerState)
{
	UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem::RequestLoadSkillData is deprecated. Use SkillDomainService instead."));
	
	// 클라이언트는 DB에서 데이터를 로드하지 않고 복제를 기다립니다.
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	if (SkillRepositoryInterface.GetInterface() && PlayerState)
	{
		// Legacy support - just call repository directly
		SkillRepositoryInterface->RequestLoadSkillsForPlayer(PlayerState);
	}
}

void USkillSubsystem::Client_OnSkillStateUpdated(USkillComponent* SkillComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem::Client_OnSkillStateUpdated is deprecated. Use SkillDomainService instead."));
	
	// Legacy support - minimal implementation
	if (SkillRepositoryInterface.GetInterface())
	{
		// The repository can perform any client-side logic, like updating local caches or data assets.
	}
}
