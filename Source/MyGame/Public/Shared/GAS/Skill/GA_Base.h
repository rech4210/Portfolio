#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SkillModule/Public/Data/FSkillContext.h"
#include "GA_Base.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MYGAME_API UGA_Base : public UGameplayAbility
{
	GENERATED_BODY()
public:
	static const FGameplayTag SkillAssetTypeTag;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skill")
	FSkillContext SkillContext;

	UPROPERTY(VisibleAnywhere)
	FVector CacheHitLocation;
	// virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
protected:
	virtual FSkillContext BuildSkillContext(const FGameplayAbilityActorInfo *ActorInfo);
	void PreProcessSkillStart(const FGameplayAbilityActorInfo* ActorInfo);
	UAbilitySystemComponent* GetTargetASC(AActor* Actor) const;
};
