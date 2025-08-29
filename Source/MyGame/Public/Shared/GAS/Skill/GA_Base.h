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

	UGA_Base();
protected:
	UFUNCTION(BlueprintCallable)
	virtual void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data){}

	UFUNCTION(BlueprintCallable)
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data){}

	UFUNCTION(BlueprintCallable)
	virtual void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData){}

	UFUNCTION(BlueprintCallable)
	virtual void OnMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData){}

	UFUNCTION(BlueprintCallable)
	virtual TArray<AActor*> GetTargetActorsByStrategy(){return TArray<AActor*>();}
	
	UPROPERTY()
	FSkillContext SkillContext;
	virtual FSkillContext BuildSkillContext(const FGameplayAbilityActorInfo *ActorInfo);
	void PreProcessSkillStart(const FGameplayAbilityActorInfo* ActorInfo);
	UAbilitySystemComponent* GetTargetASC(AActor* Actor) const;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
