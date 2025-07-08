// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillSubsystem.h"

#include "DatabaseManager.h"
#include "Repositories/SkillConfigRepository.h"
#include "Repositories/SkillStateRepository.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerState.h"

void USkillSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//db를 우선 초기화 진행
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	SkillConfigRepository = NewObject<USkillConfigRepository>(this, TEXT("SkillConfigRepository"));
	SkillConfigRepository->Initialize();
	SkillStateRepository = NewObject<USkillStateRepository>(this, TEXT("SkillStateRepository"));
	SkillStateRepository->Initialize();
}

void USkillSubsystem::Deinitialize()
{
	SkillConfigRepository = nullptr;
	SkillStateRepository = nullptr;
	Super::Deinitialize();
}

TScriptInterface<ISkillConfigRepositoryInterface> USkillSubsystem::GetSkillConfigRepository() const
{
	auto DB = GetGameInstance()->GetSubsystem<UDatabaseManager>();
	return SkillConfigRepository;
}

TScriptInterface<ISkillStateRepositoryInterface> USkillSubsystem::GetSkillStateRepository() const
{
	return SkillStateRepository;
}

void USkillSubsystem::RequestLoadSkillData(APlayerState* PlayerState)
{
	// 클라이언트는 DB에서 데이터를 로드하지 않고 복제를 기다립니다.
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	if (SkillStateRepository && PlayerState)
	{
		// 서버에서만 실행: 비동기적으로 DB 또는 외부 저장소에서 스킬 상태 데이터 로드
		if (auto* SkillComponent = PlayerState->FindComponentByClass<USkillComponent>())
		{
			// 실제 구현에서는 이 부분이 비동기 데이터베이스 호출이 될 것입니다.
			// 현재는 Mock 데이터로 시뮬레이션합니다.
			TArray<int32> MockFetchedSkillList = { 100, 101, 102, 103, 104, 105, 106, 107 };
			
			// 서버 권한이 있을 때만 SkillComponent의 복제된 프로퍼티를 수정
			if (PlayerState->HasAuthority())
			{
				SkillStateRepository->LoadSkillState(PlayerState->GetPlayerId(), *SkillComponent, MockFetchedSkillList);
			}
		}
	}
}

void USkillSubsystem::Client_OnSkillStateUpdated(USkillComponent* SkillComponent)
{
	if (SkillStateRepository && SkillComponent)
	{
		// 클라이언트 측에서 복제된 데이터를 받았을 때의 로직
		// 로컬 데이터 에셋이나 캐시를 로드·적용하고, 필요한 로직을 수행
		
		// 예: 로컬 스킬 에셋 캐시 업데이트, UI 갱신 등
		UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: Client received skill state update for %d skills"), 
			SkillComponent->GetAllSkillSlots().Num());
	}
}
