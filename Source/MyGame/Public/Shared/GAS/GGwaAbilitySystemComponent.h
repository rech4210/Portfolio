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
	bool CheckCost(const FGameplayEffectSpecHandle& CostSpecHandle);

	UPROPERTY()
	FOnEffectAssetApplied OnEffectAssetApplied;

	void ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Parameters);
	
private:
	void OnGameplayAppliedCallback(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void ProcessGameplayEffect(const FGameplayEffectSpec& Spec, bool bIsServer) const;
};
