// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MoveAbility.generated.h"

/**
 * 
 */
class AGGwaCharacter;
UCLASS()
class MYGAME_API UGA_MoveAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MoveAbility();
	UPROPERTY(EditAnywhere, Category="LineTrace")
	TSubclassOf<AActor> LineTraceRing;


	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data);
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
