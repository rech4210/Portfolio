// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../AbilityInputID.h"
#include "GA_Base.h"
#include "GA_Skill1.generated.h"
class UBuffDataAsset;
class USkillDataAsset;
class UPrimaryDataAsset;
UCLASS()
class MYGAME_API UGA_Skill1 : public UGA_Base
{
	GENERATED_BODY()

public:
	UGA_Skill1();
	UPROPERTY()
	EAbilityInputID AbilityInputID;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UBuffDataAsset> BuffDataAsset;
	UPROPERTY()
	TArray<UBaseDataAsset*> DataAssets;
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	// FGameplayTagContainer CooldownTags;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UFUNCTION()
	void OnMontageEnded();
	UFUNCTION()
	void OnMontageInterrupted();
};
