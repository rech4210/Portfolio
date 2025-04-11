// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseDataAsset.h"
#include "BuffDataAsset.generated.h"

class UGameplayEffect;
/**
 * 
 */
UCLASS()
class MYGAME_API UBuffDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsDebuff;
	
};
