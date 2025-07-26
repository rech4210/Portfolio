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
	FString CurrentMapName = GetWorld()->GetMapName();
	
	UE_LOG(LogTemp, Warning, TEXT("=== GGwaGameInstance::HandleMapLoading ==="));
	UE_LOG(LogTemp, Warning, TEXT("PIE: %s, NetMode: %d, DedicatedServer: %s"), 
		bIsPIEEnvironment ? TEXT("Yes") : TEXT("No"),
		(int32)NetMode,
		bIsActualDedicatedServer ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Warning, TEXT("Current Map: %s"), *CurrentMapName);
	
	// Check if we're already in ThirdPersonMap (서버가 잘못된 맵에서 시작된 경우)
	if (CurrentMapName.Contains(TEXT("ThirdPersonMap")))
	{
		UE_LOG(LogTemp, Error, TEXT("PROBLEM: Server started with ThirdPersonMap instead of LoginLevel!"));
		UE_LOG(LogTemp, Error, TEXT("This causes clients to auto-travel to ThirdPersonMap"));
		UE_LOG(LogTemp, Error, TEXT("Check PIE settings or Blueprint GameMode logic"));
	}

	// GameInstance는 초기 환경 설정과 로깅만 담당
	// 실제 맵 전환 로직은 GameMode에서 플레이어 수 등을 고려하여 처리
	
	if (bIsPIEEnvironment)
	{
		UE_LOG(LogTemp, Warning, TEXT("PIE Environment: LoginLevel로 시작, 각 클라이언트가 개별 인증 수행"));
	}
	else if (bIsActualDedicatedServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dedicated Server: LoginLevel로 시작, GameMode에서 플레이어 상태에 따라 맵 전환 결정"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client/ListenServer: LoginLevel에서 인증 프로세스 시작"));
	}
	
	// Note: 맵 전환 로직은 AGGwaGameMode에서 플레이어 접속 상황을 보고 결정
}
