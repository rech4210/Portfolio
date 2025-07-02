// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISkillRepositoryInterface.h"
#include "UObject/Object.h"
#include "SkillStateRepository.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillStateRepository : public UObject, public ISkillStateRepositoryInterface {
	GENERATED_BODY()

public:
	virtual bool LoadSkillState(int32 PlayerId, USkillComponent* SkillComponentToPopulate) override;
	virtual bool SaveSkillState(int32 PlayerId, const USkillComponent* SkillComponentToSave) override;
};
