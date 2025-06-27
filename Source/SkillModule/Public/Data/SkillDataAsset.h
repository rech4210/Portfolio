// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "GameSharedModule/Public/Data/BaseDataAsset.h"
#include "GameSharedModule/Public/Data/BuffDataAsset.h"
#include "FSkillShapeConfig.h"
#include "SkillInputSlot.h"
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

	//EIC키 와 연동을 위한 스킬 인덱스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	ESkillInputSlot SkillSlotIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 SkillID = 999;
	
	// 타겟 선택 전략 (예: 단일 타겟, 범위, 셀프 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TSubclassOf<USkillTargetBase> TargetStrategyClass;

	// 실제로 적용되는 GameplayEffect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GEClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float CoolTime = 0.f;

	// CoolTime GameplayEffect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_CoolTimeClass;

	// CoolTime GameplayEffect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_CueClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float CueDuration = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	TArray<UBuffDataAsset*> AppliedBuffs;

	// 시전 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UAnimMontage* CastMontage;

	// 시전 효과음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	USoundBase* CastSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FGameplayTag SkillTag;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	// TArray<FSkillDataTypeContainer> SkillData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<UBuffDataAsset> BuffData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	ESkillInputSlot InputSlot;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override {
		return FPrimaryAssetId("Skill", GetFName());
	}
}; 