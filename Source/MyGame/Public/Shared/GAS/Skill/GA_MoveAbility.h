
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

	UPROPERTY(EditAnywhere, Category="GA")
	float CoolTime = 0.3f;

	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data);
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
