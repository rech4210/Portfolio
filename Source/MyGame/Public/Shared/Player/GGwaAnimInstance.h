// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GGwaAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UGGwaAnimInstance : public UAnimInstance {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	bool bIsPlayingMontage;
};
