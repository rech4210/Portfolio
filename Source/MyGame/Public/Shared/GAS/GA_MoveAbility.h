// Fill out your copyright notice in the Description page of Project Settings.

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
	UPROPERTY(EditAnywhere, Category="LineTrace")
	TSubclassOf<AActor> LineTraceRing;

	// UPROPERTY(VisibleAnywhere, Category="LineTrace")
	// TObjectPtr<AActor> Actor;
	// UPROPERTY(VisibleAnywhere, Category="LineTrace")
	// TObjectPtr<AGGwaCharacter> Character;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
