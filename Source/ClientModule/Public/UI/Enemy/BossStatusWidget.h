// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "BossStatusWidget.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UBossStatusWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	void UpdateBossWidget(const FBossDataStruct& Data);
};
