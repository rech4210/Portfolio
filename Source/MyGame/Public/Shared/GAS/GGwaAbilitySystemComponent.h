// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GGwaAbilitySystemComponent.generated.h"
class ULocalDataBaseLoader;
class UBaseDataAsset; 


class UCombinedAbilityDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectAssetApplied, UBaseDataAsset*, DataAsset);

UCLASS()
class MYGAME_API UGGwaAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	
	UGGwaAbilitySystemComponent();
	virtual void BeginPlay() override;
	// check cost for variable cost with tag or enum
	bool CheckCost(const FGameplayEffectSpecHandle& CostSpecHandle);

	UPROPERTY()
	FOnEffectAssetApplied OnEffectAssetApplied;

	UPROPERTY()
	TObjectPtr<ULocalDataBaseLoader> LocalDataBaseLoader;
	
	void ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Parameters);
	
private:
	void OnGameplayAppliedCallback(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void ProcessGameplayEffect(const FGameplayEffectSpec& Spec, bool bIsServer) const;
};
