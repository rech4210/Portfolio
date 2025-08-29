
#pragma once

#include "CoreMinimal.h"
#include "GameSharedModule/Public/Data/BuffDataAsset.h"
#include "FSkillShapeConfig.h"
#include "SkillInputSlot.h"
#include "Animation/AnimMontage.h"
#include "Abilities/GameplayAbility.h"
#include "SkillDataAsset.generated.h"

class USkillTargetBase;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FSkillShapeConfig SkillShapeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FString SkillSlotKey;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	int SkillSlotIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 SkillID = 999;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TSubclassOf<USkillTargetBase> TargetStrategyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TArray<TSubclassOf<UGameplayEffect>> GEClasses;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float CoolTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_CueClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float CueDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	float CostAmount = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	TArray<UBuffDataAsset*> AppliedBuffs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UAnimMontage* CastMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	USoundBase* CastSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UBuffDataAsset> BuffData;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override {
		return FPrimaryAssetId("Skill", GetFName());
	}
}; 