// Shared/Mode/BaseGameMode.cpp

#include "Shared/Mode/BaseGameMode.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"

ABaseGameMode::ABaseGameMode()
{
	// 공통 클래스 세팅
	PlayerControllerClass = AGGwaPlayerController::StaticClass();
	PlayerStateClass    = AGGwaPlayerState::StaticClass();
	DefaultPawnClass    = AGGwaCharacter::StaticClass();
}

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();
	// 서버 전용 매니저 초기화 훅 호출
	InitializeServerManagers();
}

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("PostLogin: NewPlayer is null"));
		return;
	}

	// 로그인된 플레이어 상태 확인
	if (AGGwaPlayerState* State = NewPlayer->GetPlayerState<AGGwaPlayerState>())
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerState found: %s"), *State->GetPlayerName());
	}

	// 이미 Pawn이 스폰됐는지 검사
	if (NewPlayer->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player already has Pawn"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Pawn not spawned yet"));
	}
}

void ABaseGameMode::HandleAbilityActivated(const AActor* InstigatorActor,
										   FName AbilityName,
										   const FVector& TargetLocation)
{
	// 서버 로그 전용이지만, 클라이언트 호출 시엔 로그만 남음
	UE_LOG(LogTemp, Log, TEXT("[Ability] %s activated %s at %s"),
		   *InstigatorActor->GetName(),
		   *AbilityName.ToString(),
		   *TargetLocation.ToString());
}
