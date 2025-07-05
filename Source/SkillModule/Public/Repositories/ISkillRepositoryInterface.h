// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISkillRepositoryInterface.generated.h"

class USkillDataAsset;
class USkillComponent;

/**
 * 스킬의 정의(DataAsset)를 로드하기 위한 리포지토리 인터페이스입니다.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USkillConfigRepositoryInterface : public UInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillConfigRepositoryInterface
{
	GENERATED_BODY()

public:
	virtual void LoadSkillDefinitions(TArray<USkillDataAsset*>& OutSkillDefinitions) = 0;
};

/**
 * 플레이어의 스킬 상태(슬롯 정보, 쿨다운 등)를 로드하고 저장하기 위한 리포지토리 인터페이스입니다.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USkillStateRepositoryInterface : public UInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillStateRepositoryInterface
{
	GENERATED_BODY()

public:
	virtual bool LoadSkillState(int32 PlayerInformation, USkillComponent& SkillComponentToPopulate, TArray<int32> fetchedSkillList) = 0;

	virtual bool SaveSkillState(int32 PlayerInformation, const USkillComponent* SkillComponentToSave, TArray<int32> SkillPayloadList) = 0;
}; 