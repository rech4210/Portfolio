// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseDataAsset.h"
#include "SkillDataAsset.generated.h"

class UGameplayEffect;
/**
 * 
 */
UCLASS()
class MYGAME_API USkillDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> SkillEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* SkillMontage;
};
