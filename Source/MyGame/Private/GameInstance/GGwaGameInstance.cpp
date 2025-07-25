// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/GGwaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HAL/Platform.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"

UGGwaGameInstance::UGGwaGameInstance()
{
}

void UGGwaGameInstance::Init() {
	Super::Init();

	// 맵 로딩은 Init 단계에서는 아직 World가 완전히 초기화되지 않았을 수 있어서,
	// 약간의 지연을 줘서 안전하게 처리
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGGwaGameInstance::HandleMapLoading, 0.1f, false);
}

void UGGwaGameInstance::HandleMapLoading() {
	if (!GetWorld()) return;

	// PIE 환경 및 네트워크 모드 정확히 감지
	bool bIsPIEEnvironment = GetWorld()->WorldType == EWorldType::PIE;
	bool bIsActualDedicatedServer = IsDedicatedServerInstance();
	ENetMode NetMode = GetWorld()->GetNetMode();
	
	UE_LOG(LogTemp, Warning, TEXT("GGwaGameInstance: PIE: %s, NetMode: %d, DedicatedServer: %s"), 
		bIsPIEEnvironment ? TEXT("Yes") : TEXT("No"),
		(int32)NetMode,
		bIsActualDedicatedServer ? TEXT("Yes") : TEXT("No"));

	// PIE 환경에서는 서버 Travel 사용하지 않음 (클라이언트들이 각자 인증 필요)
	if (bIsPIEEnvironment)
	{
		UE_LOG(LogTemp, Warning, TEXT("PIE 환경: 각 클라이언트가 개별적으로 LoginLevel에서 인증을 시작합니다."));
		// PIE에서는 ServerTravel 하지 않음 - 각 클라이언트가 개별적으로 처리
		return;
	}

	// 실제 전용 서버 환경에서만 서버 Travel 실행
	if (bIsActualDedicatedServer && NetMode == NM_DedicatedServer)
	{
		// 개발/테스트 모드 감지
		bool bDevelopmentMode = FParse::Param(FCommandLine::Get(), TEXT("DevMode")) ||
			FParse::Param(FCommandLine::Get(), TEXT("TestMode")) ||
			UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG;
		
		if (bDevelopmentMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("전용 서버 개발 환경: 테스트를 위해 MainMap으로 자동 이동합니다."));
			GetWorld()->ServerTravel(GetGameWorldURL());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("전용 서버 운영 환경: JWT 인증 후에만 게임 월드 접근 가능. LoginLevel에서 대기합니다."));
			// 실제 게임 월드 접근은 AuthSubsystem::OnGameDataLoaded()에서 토큰 검증 후에만 허용
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("클라이언트/리슨서버: LoginLevel에서 인증 프로세스를 시작합니다."));
		// 클라이언트는 LoginLevel에서 시작하여 인증 UI 표시
	}
}
