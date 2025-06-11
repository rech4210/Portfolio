// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BossAttack.generated.h"

class AAttackProjectile;
/**
 * 
 */
UCLASS()
class MYGAME_API UGA_BossAttack : public UGameplayAbility {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AActor> Projectile;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<AAttackProjectile> SpawnedProjectile;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FVector ProjectileOffset;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
