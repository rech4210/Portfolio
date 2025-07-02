// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISkillRepositoryInterface.h"
#include "UObject/Object.h"
#include "SkillConfigRepository.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillConfigRepository : public UObject, public ISkillConfigRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void LoadSkillDefinitions(TArray<USkillDataAsset*>& OutSkillDefinitions) override;
};
