// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GGwaAbilitySystemComponent.generated.h"
class ULocalDataBaseLoader;
class UBaseDataAsset; 
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownTagChanged, FGameplayTag, CooldownTag, float, TimeRemaining);


class UCombinedAbilityDataAsset;
UCLASS()
class MYGAME_API UGGwaAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	
	UGGwaAbilitySystemComponent();
	// UPROPERTY(BlueprintAssignable)
	// FOnCooldownTagChanged OnCooldownTagChanged;
	virtual void BeginPlay() override;
	UPROPERTY()
	TObjectPtr<ULocalDataBaseLoader> LocalDataBaseLoader;
private:
	void OnGameplayAppliedCallback(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void ProcessGameplayEffect(const FGameplayEffectSpec& Spec, bool bIsServer) const;
	// TObjectPtr<UCombinedAbilityDataAsset> CachedAbilityDataAsset;
};
