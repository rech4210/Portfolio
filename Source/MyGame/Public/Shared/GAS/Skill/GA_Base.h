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

protected:
	virtual FSkillContext BuildSkillContext(const FGameplayAbilityActorInfo *ActorInfo);
	virtual TArray<AActor*> GetTargetActorsByStrategy(FSkillContext SkillContext){return nullptr;}
	void PreProcessSkillStart(const FGameplayAbilityActorInfo* ActorInfo);
	UAbilitySystemComponent* GetTargetASC(AActor* Actor) const;
};
