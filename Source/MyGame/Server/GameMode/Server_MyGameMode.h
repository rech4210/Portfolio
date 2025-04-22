// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Server_MyGameMode.generated.h"

class AGGwaCharacter;
class AGGwaPlayerState;
class AGGwaPlayerController;
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

	virtual void PostLogin(APlayerController* NewPlayer) override;
	UFUNCTION()
	static void HandleAbilityActivated(const AActor* InstigatorActor, FName AbilityName, const FVector& TargetLocation);
};
