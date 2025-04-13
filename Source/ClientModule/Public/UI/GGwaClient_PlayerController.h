// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "GGwaClient_PlayerController.generated.h"

class UBaseDataAsset;
class AGGwaHUD;
class UGGwaWidget;
class UGGwaAttributeSet;
/**
 * 
 */
UCLASS(Blueprintable)
class CLIENTMODULE_API AGGwaClient_PlayerController : public AGGwaPlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void InitClientWidget() override;
	virtual void GetDataFromAbility(UBaseDataAsset* Data) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UGGwaWidget> WidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGGwaHUD> GGwaHUD;
};
