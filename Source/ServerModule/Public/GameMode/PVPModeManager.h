// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BattleModeBase.h"
#include "PVPModeManager.generated.h"

/**
 * 
 */
UCLASS()
class SERVERMODULE_API UPVPModeManager : public UBattleModeBase {
	GENERATED_BODY()
public:
	virtual void InitMode() override;
	virtual void StartBattle() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndBattle() override;
	virtual void ResetBattle() override;
	virtual TStatId GetStatId() const override;
};
