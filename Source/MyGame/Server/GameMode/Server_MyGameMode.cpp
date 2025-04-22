// Fill out your copyright notice in the Description page of Project Settings.


#include "Server_MyGameMode.h"

#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"


AServer_MyGameMode::AServer_MyGameMode() {
	PlayerController = AGGwaPlayerController::StaticClass();
	PlayerState = AGGwaPlayerState::StaticClass();
	Character = AGGwaCharacter::StaticClass();
}

void AServer_MyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	AGGwaPlayerState* State = NewPlayer->GetPlayerState<AGGwaPlayerState>();
	// 플레이어 데이터 호출 Player Default Settings
	
}

void AServer_MyGameMode::HandleAbilityActivated(const AActor* Instigator, FName AbilityName, const FVector& TargetLocation){
	//서버 사이드 로깅
	UE_LOG(LogTemp, Log, TEXT("[Server] %s activated ability %s at %s"),
		*Instigator->GetName(),
		*AbilityName.ToString(),
		*TargetLocation.ToString());
}