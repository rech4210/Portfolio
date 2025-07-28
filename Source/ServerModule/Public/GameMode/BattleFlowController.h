// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BattleModeBase.h"
#include "../../../MyGame/Public/Shared/Mode/ModeType.h"
#include "BattleFlowController.generated.h"

/**
 * 
 */
UCLASS()
class SERVERMODULE_API UBattleFlowController : public UObject {
	GENERATED_BODY()
public:
	void InitBattleMode(EModeType ModeType);
	void SetupBattleMode(EModeType ModeType);
private:
	UPROPERTY()
	TObjectPtr<UBattleModeBase> BattleMode = nullptr;
};
