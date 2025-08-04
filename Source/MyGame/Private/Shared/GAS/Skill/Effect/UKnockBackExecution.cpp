

#include "Shared/GAS/Skill/Effect/UKnockBackExecution.h"
#include "AbilitySystemComponent.h"
#include "GameSharedModule/Public/Data/EGasDataType.h"
#include "GameSharedModule/Public/Enum/EPlayerState.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"

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

	FVector KnockbackDir = (TargetActor->GetActorLocation() - SourceActor->GetActorLocation()).GetSafeNormal();

	TargetCharacter->LaunchCharacter(KnockbackDir * KnockbackStrength + FVector(0, KnockbackHeight, KnockbackDistance), true, true);
}

