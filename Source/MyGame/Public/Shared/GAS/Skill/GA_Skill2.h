// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_Base.h"
#include "Shared/GAS/Interface/ICustomAbilityTaskInterface.h"
#include "GA_Skill2.generated.h"

class UBuffDataAsset;
class USkillDataAsset;
class AGameplayAbilityTargetActor;
UCLASS()
class MYGAME_API UGA_Skill2 : public UGA_Base, public ICustomAbilityTaskInterface{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<AGameplayAbilityTargetActor> AbilityTargetActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Data")
	TObjectPtr<UBuffDataAsset> BuffDataAsset;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	virtual UAbilityTask_WaitTargetData* GetTargetDataTask() override;
	
	UFUNCTION()
	virtual void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data) override;
	UFUNCTION()
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data) override;
};
