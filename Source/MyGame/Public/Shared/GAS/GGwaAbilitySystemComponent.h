// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GGwaAbilitySystemComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownTagChanged, FGameplayTag, CooldownTag, float, TimeRemaining);

/**
 * 
 */
UCLASS()
class MYGAME_API UGGwaAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	
	UGGwaAbilitySystemComponent();
	UPROPERTY(BlueprintAssignable)
	FOnCooldownTagChanged OnCooldownTagChanged;
	virtual void BeginPlay() override;
private:
	void OnGameplayAppliedCallback(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
};
