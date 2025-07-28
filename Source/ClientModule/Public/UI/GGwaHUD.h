// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/Widget/GGwaWidget.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "GGwaHUD.generated.h"

class UBossStatusWidget;
class UBaseDataAsset;

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API AGGwaHUD : public AHUD
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION()
	void HandleAbilityDataApplied(UBaseDataAsset* Data);
	UFUNCTION()
	void HandleBossDataReceived(const FBossDataStruct& BossData);

public:
	UGGwaWidget* GetBaseWidget() const;
	UBossStatusWidget* GetBossWidget() const;

	void SetBaseWidget(UGGwaWidget* widget);
	void SetBossWidget(UBossStatusWidget* widget);
private:
	UPROPERTY()
	TObjectPtr<UGGwaWidget> BaseWidget;
	UPROPERTY()
	TObjectPtr<UBossStatusWidget> BossWidget;
};
