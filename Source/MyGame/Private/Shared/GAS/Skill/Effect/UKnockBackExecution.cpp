// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Skill/Effect/UKnockBackExecution.h"
#include "AbilitySystemComponent.h"
#include "Shared/Data/EGasDataType.h"
#include "Shared/Player/EPlayerState.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"

UKnockBackExecution::UKnockBackExecution() {
	
}

void UKnockBackExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                 FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const {
	
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;

	if (!TargetActor || !SourceActor)
		return;

	AGGwaCharacter* TargetCharacter = Cast<AGGwaCharacter>(TargetActor);
	if (!TargetCharacter)
		return;

	// 넉백 방향과 세기
	FVector KnockbackDir = (TargetActor->GetActorLocation() - SourceActor->GetActorLocation()).GetSafeNormal();

	TargetCharacter->LaunchCharacter(KnockbackDir * KnockbackStrength + FVector(0, KnockbackHeight, KnockbackDistance), true, true);
}

