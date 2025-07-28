
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

	//EIC???� ?�동???�한 ?�킬 ?�덱??
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	ESkillInputSlot SkillSlotIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 SkillID = 999;
	
	// ?��??�택 ?�략 (?? ?�일 ?��? 범위, ?�????
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TSubclassOf<USkillTargetBase> TargetStrategyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GA")
	TSubclassOf<UGameplayAbility> AbilityClass;
	
	// ?�제�??�용?�는 GameplayEffect ?�래??
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GEClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float CoolTime = 0.f;

	// CoolTime GameplayEffect ?�래??
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_CoolTimeClass;

	// CoolTime GameplayEffect ?�래??
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_CueClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float CueDuration = 0.f;

	/** The attribute to use as a cost for this skill (e.g., Mana, Stamina). */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	// FGameplayAttribute CostAttribute;

	/** The amount of the attribute to consume. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	float CostAmount = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	TArray<UBuffDataAsset*> AppliedBuffs;

	// ?�전 ?�니메이??
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UAnimMontage* CastMontage;

	// ?�전 ?�과??
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	USoundBase* CastSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillTag;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	// TArray<FSkillDataTypeContainer> SkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UBuffDataAsset> BuffData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	ESkillInputSlot InputSlot;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override {
		return FPrimaryAssetId("Skill", GetFName());
	}
}; 