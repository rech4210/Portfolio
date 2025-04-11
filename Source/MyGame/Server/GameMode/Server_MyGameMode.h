// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Shared/GAS/GA_Skill1.h"
#include "Shared/GAS/GE_Test.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Server_MyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API AServer_MyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AServer_MyGameMode();
	UPROPERTY()
	TSubclassOf<AGGwaPlayerController> PlayerController;

	UPROPERTY()
	TSubclassOf<AGGwaPlayerState> PlayerState;

	UPROPERTY()
	TSubclassOf<AGGwaCharacter> Character;


// 	FGameplayAbilitySpec(
// 	TSubclassOf<UGameplayAbility> Ability,  // 어떤 능력?
// 	int32 Level,                            // 능력 레벨
// 	int32 InputID,                          // 입력 슬롯 (예: Q=0, E=1 등)
// 	UObject* SourceObject = nullptr         // 출처 객체 (무기, 아이템 등)
// )
	UPROPERTY()
	TSubclassOf<UGA_Skill1> Test_Ability;
	UPROPERTY()
	TSubclassOf<UGE_Test> Test_Effect;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
