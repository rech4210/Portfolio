
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "UKnockBackExecution.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UKnockBackExecution : public UGameplayEffectExecutionCalculation {
	GENERATED_BODY()
public:
	UKnockBackExecution();

	UPROPERTY(EditAnywhere)
	float KnockbackStrength = 1000.f;
	UPROPERTY(EditAnywhere)
	float KnockbackDistance = 300.f;
	UPROPERTY(EditAnywhere)
	float KnockbackHeight = 30.f;
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
