// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillModule/Public/Data/SkillInputSlot.h"
#include "UObject/Object.h"
#include "USkillHelper.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillHelper : public UObject {
	GENERATED_BODY()
public:
	static ESkillInputSlot GetKeyFromIndex(int32 SlotIndex);
	static int GetValueFromKey(ESkillInputSlot SlotIndex);
	static FString GetKeyString(ESkillInputSlot InputSlot);
}; 