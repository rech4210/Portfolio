// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseDataAsset.h"
#include "FSkillDataTypeContainer.h"
#include "SkillDataAsset.generated.h"


class USkillTargetBase;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class MYGAME_API USkillDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()
public:
	// 쿨타임 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float Cooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FSkillShapeConfig SkillShapeConfig;
	
	// 타겟 선택 전략 (예: 단일 타겟, 범위, 셀프 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TSubclassOf<USkillTargetBase> TargetStrategyClass;

	// 실제로 적용되는 GameplayEffect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GEClass;

	// 시전 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UAnimMontage* CastMontage;

	// 시전 효과음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	USoundBase* CastSound;
};
