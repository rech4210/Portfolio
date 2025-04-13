// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_GroundArea.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API USkillTarget_GroundArea : public USkillTargetBase {
	GENERATED_BODY()
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
