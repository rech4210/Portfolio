// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISkillRepositoryInterface.h"
#include "UObject/Object.h"
#include "SkillConfigRepository.generated.h"

class UDatabaseManager;
/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillConfigRepository : public UObject, public ISkillConfigRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void Initialize() override;
	virtual void LoadSkillDefinitions(TArray<USkillDataAsset*>& OutSkillDefinitions) override;
private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};
