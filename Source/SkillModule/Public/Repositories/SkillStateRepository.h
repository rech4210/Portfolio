// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISkillRepositoryInterface.h"
#include "UObject/Object.h"
#include "SkillStateRepository.generated.h"

class ULocalDataBaseLoader;
class UDatabaseManager;
/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillStateRepository : public UObject, public ISkillStateRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void Initialize() override;
	virtual bool LoadSkillState(int32 PlayerInformation, USkillComponent& SkillComponentToPopulate, TArray<int32> fetchedSkillList) override;
	virtual bool SaveSkillState(int32 PlayerInformation, const USkillComponent* SkillComponentToSave, TArray<int32> SkillPayloadList) override;
	virtual bool LoadSkillStateFromDB_Temp(int32 PlayerInformation, const TArray<int32>& fetchedSkillList) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};
