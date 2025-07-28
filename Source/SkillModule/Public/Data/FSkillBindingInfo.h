#pragma once
#include "AbilityInputID.h"
#include "SkillDataAsset.h"
#include "InputCoreTypes.h"
#include "InputAction.h"
#include "FSkillBindingInfo.generated.h"

USTRUCT(BlueprintType)
struct FSkillBindingInfo {
	GENERATED_BODY()

	// ���� ���� �ĺ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	FGuid SlotId;

	// UI���� ���� �ε��� (0���� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	int32 SlotIndex;

	// AbilitySystem�� ����� �Է� ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	EAbilityInputID AbilityInputID;

	// Enhanced Input Action �ڻ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	UInputAction* InputAction;

	// ���� ���ε��� Ű
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	FKey BindedKey;

	// ���Կ� ������ ��ų ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	USkillDataAsset* SkillData;
};
