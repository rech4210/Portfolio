// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Data/BaseDataAsset.h"
#include "Shared/Data/FSkillDataTypeContainer.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTargetBase.h"
#include "BossSkillDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UBossSkillDataAsset : public UBaseDataAsset {
	GENERATED_BODY()
public:
	//Boss의 경우 BB를 사용할수도 있음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float CoolTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FSkillShapeConfig SkillShapeConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 SkillID = 999;
	
	// 타겟 선택 전략 (예: 단일 타겟, 범위, 셀프 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TSubclassOf<USkillTargetBase> TargetStrategyClass;

	// 실제로 적용되는 GameplayEffect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GEClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TArray<FPrimaryAssetId> AppliedBuffs;

	// 시전 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UAnimMontage* CastMontage;

	// 시전 효과음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	USoundBase* CastSound;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override {
		return FPrimaryAssetId("Skill", GetFName());
	}
};
