﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_TargetActor.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API USkillTarget_TargetActor : public USkillTargetBase {
	GENERATED_BODY()
public:
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
