// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Shared/GAS/SkillTargetPolicy/FSkillContext.h"
#include "GA_Base.generated.h"


UCLASS(BlueprintType, Blueprintable)
class MYGAME_API UGA_Base : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skill")
	FSkillContext SkillContext;

	UPROPERTY(VisibleAnywhere)
	FVector CacheHitLocation;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:
	virtual FSkillContext BuildSkillContext(const FGameplayAbilityActorInfo *ActorInfo);
};
