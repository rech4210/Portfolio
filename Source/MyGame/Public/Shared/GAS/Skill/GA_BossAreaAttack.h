// @Needmodifi
#pragma once

#include "CoreMinimal.h"
#include "GA_Base.h"
#include "GA_BossAreaAttack.generated.h"

class USkillDataAsset;

UCLASS()
class MYGAME_API UGA_BossAreaAttack : public UGA_Base
{
    GENERATED_BODY()

public:
    UGA_BossAreaAttack();


    UPROPERTY(EditAnywhere)
    TSubclassOf<UGameplayEffect> TagEffect;
    UPROPERTY(EditAnywhere)
    TObjectPtr<USkillDataAsset> SkillDataAsset;
    	
    UPROPERTY(EditAnywhere)
    float KnockbackTagMagnitude = 0.f;
protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    UFUNCTION()
    void PerformAreaAttack();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    float AttackRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    float DamageAmount;

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    float DelayBeforeAttack;

};