// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Shared/GAS/SkillTargetPolicy/FSkillContext.h"
#include "SkillTargetBase.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API USkillTargetBase : public UObject
{
	GENERATED_BODY()

public:
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) {
		UE_LOG(LogTemp, Warning, TEXT("SkillTarget: Base"));
		return TArray<AActor*>();
	}
	virtual void DebugSkillShape(const UWorld* World, const FVector& StartLocation, const FVector& EndLocation, const FSkillShapeConfig& Config) const;
};
