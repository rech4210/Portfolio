// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_Projectile.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API USkillTarget_Projectile : public USkillTargetBase {
	GENERATED_BODY()
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
