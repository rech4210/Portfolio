#pragma once

#include "CoreMinimal.h"
#include "SkillModule/Public/Data/AbilityInputID.h"
#include "GA_Base.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Shared/GAS/AbilityTask/GGwaPlayMontageAndWaitForEvent.h"
#include "GA_Skill1.generated.h"

class USkillDataAsset;
class USkillTargetBase;

UCLASS()
class MYGAME_API UGA_Skill1 : public UGA_Base
{
	GENERATED_BODY()
public:
	UGA_Skill1();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	UPROPERTY()
	TArray<AActor*>	 TargetActors;
	virtual TArray<AActor*> GetTargetActorsByStrategy() override;
	virtual void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData) override;
	virtual void OnMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData) override;
	
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data) override;
	virtual void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data) override;
private:
	FVector HitPoint;
	UPROPERTY()
	USkillTargetBase* TargetingStrategy;
};
