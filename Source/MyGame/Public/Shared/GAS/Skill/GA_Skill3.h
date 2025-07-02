#pragma once

#include "CoreMinimal.h"
#include "SkillModule/Public/Data/AbilityInputID.h"
#include "GA_Base.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Shared/GAS/AbilityTask/GGwaPlayMontageAndWaitForEvent.h"
#include "GA_Skill3.generated.h"

class UBuffDataAsset;
class USkillDataAsset;
class UPrimaryDataAsset;
class USkillTargetBase;

UCLASS()
class MYGAME_API UGA_Skill3 : public UGA_Base
{
	GENERATED_BODY()

public:
	EAbilityInputID AbilityInputID;

	/** 스킬 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset;
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData);
	FVector HitPoint;
};
