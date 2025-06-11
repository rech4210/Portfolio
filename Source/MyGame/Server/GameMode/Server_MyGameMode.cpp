// Fill out your copyright notice in the Description page of Project Settings.


#include "Server_MyGameMode.h"

#include "Shared/AI/BossCharacter.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"


//ServerModule로 옮겨야함.
AServer_MyGameMode::AServer_MyGameMode() {
	PlayerControllerClass = AGGwaPlayerController::StaticClass();
	PlayerStateClass = AGGwaPlayerState::StaticClass();
	DefaultPawnClass = AGGwaCharacter::StaticClass();
}

void AServer_MyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	if (HasAuthority()) {
		// 플레이어 데이터 호출 Player Default Settings, 데이터 로딩
		if (!NewPlayer) {
			UE_LOG(LogTemp, Error, TEXT("NewPlayer is null"));
		}
	
		AGGwaPlayerState* State = NewPlayer->GetPlayerState<AGGwaPlayerState>();

		if (AGGwaCharacter* GGwaCharacter = Cast<AGGwaCharacter>(NewPlayer->GetPawn())) {
			UE_LOG(LogTemp, Warning, TEXT("Player already exists"));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Player is not spawned in postlogin"));
		}
	}
}

void AServer_MyGameMode::HandleAbilityActivated(const AActor* Instigator, FName AbilityName, const FVector& TargetLocation){
	//서버 사이드 로깅
	UE_LOG(LogTemp, Log, TEXT("[Server] %s activated ability %s at %s"),
		*Instigator->GetName(),
		*AbilityName.ToString(),
		*TargetLocation.ToString());
}