// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerReactionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UPlayerReactionComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UFUNCTION()
	void Initialize(UGGwaAbilitySystemComponent* ASC);

	UFUNCTION()
	void HandleGameplayCue(const FGameplayTag EventTag,EGameplayCueEvent::Type CueType, const FGameplayCueParameters& Params);
	void ExecuteDeadReaction();
private:
	void ExecuteKnockback(const FVector& Velocity);
};
