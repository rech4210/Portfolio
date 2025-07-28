// @Needmodifi
#pragma once
#include "AbilityInputID.h"
#include "SkillDataAsset.h"
#include "InputCoreTypes.h"
#include "InputAction.h"
#include "FSkillBindingInfo.generated.h"

USTRUCT(BlueprintType)
struct FSkillBindingInfo {
	GENERATED_BODY()

	// 슬롯 고유 식별자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	FGuid SlotId;

	// UI상의 슬롯 인덱스 (0부터 시작)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	int32 SlotIndex;

	// AbilitySystem에 등록할 입력 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	EAbilityInputID AbilityInputID;

	// Enhanced Input Action 자산
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	UInputAction* InputAction;

	// 현재 바인딩된 키
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	FKey BindedKey;

	// 슬롯에 배정된 스킬 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	USkillDataAsset* SkillData;
};
