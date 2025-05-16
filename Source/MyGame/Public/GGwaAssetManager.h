// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "GGwaAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UGGwaAssetManager : public UAssetManager {
	GENERATED_BODY()
public:
	static const FPrimaryAssetType SkillType;

	virtual void StartInitialLoading() override;
	virtual void FinishInitialLoading() override;
};
