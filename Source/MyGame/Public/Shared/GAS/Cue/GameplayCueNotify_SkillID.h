// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayCueNotify_SkillID.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UGameplayCueNotify_SkillID : public UGameplayCueNotify_Static {
	GENERATED_BODY()
public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
