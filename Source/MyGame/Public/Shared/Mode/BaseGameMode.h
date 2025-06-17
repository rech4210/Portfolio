// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Shared/Mode/ModeType.h"
#include "BaseGameMode.generated.h"

class AGGwaCharacter;
class AGGwaPlayerController;
class AGGwaPlayerState;


/**
 * 
 */
UCLASS()
class MYGAME_API ABaseGameMode : public AGameMode {
	GENERATED_BODY()
public:
	ABaseGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION()
	static void HandleAbilityActivated(const AActor* InstigatorActor,
									   FName AbilityName,
									   const FVector& TargetLocation);
protected:
	// 서버 전용 매니저 초기화 훅 (기본 구현은 빈 동작)
	virtual void InitializeServerManagers() {}

	virtual void BeginPlay() override;

	virtual void RequestFlowControllerInit(EModeType ModeType) {}
	
};
