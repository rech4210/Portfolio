#pragma once
#include "AbilityInputID.h"
#include "SkillDataAsset.h"
#include "InputCoreTypes.h"
#include "InputAction.h"
#include "FSkillBindingInfo.generated.h"

USTRUCT(BlueprintType)
struct FSkillBindingInfo {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	FGuid SlotId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	int32 SlotIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	EAbilityInputID AbilityInputID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	UInputAction* InputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	FKey BindedKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Binding")
	USkillDataAsset* SkillData;
};
