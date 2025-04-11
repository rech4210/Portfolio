// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/Widget/GGwaWidget.h"
#include "GGwaHUD.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API AGGwaHUD : public AHUD
{
	GENERATED_BODY()
	
	
public:
	UGGwaWidget* GetBaseWidget() const;
	void SetBaseWidget(UGGwaWidget* widget);
private:
	UPROPERTY()
	TObjectPtr<UGGwaWidget> BaseWidget;
};
