// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginGameMode.h"
#include "Player/LoginPlayerController.h"

constexpr static float GAME_MODE_FREQUENCY = 3.0f;
ALoginGameMode::ALoginGameMode() {
	// 이 게임 모드의 기본 플레이어 컨트롤러를 LoginPlayerController로 설정합니다.
	PlayerControllerClass = ALoginPlayerController::StaticClass();

	// 로그인 화면에서는 플레이어가 조종할 캐릭터나 HUD가 필요 없습니다.
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void ALoginGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	static float LogTimer = 0.0f;
	LogTimer += DeltaSeconds;
	if (LogTimer >= GAME_MODE_FREQUENCY)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client is ALIVE. Current GameMode is [ %s ] running in map [ %s ]"), *GetName(), *GetWorld()->GetName());
		LogTimer = 0.0f;
	}
}