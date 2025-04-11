// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SetWidgetDataInterface.h"
#include "ItemSetWidget.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UItemSetWidget : public UUserWidget, public ISetWidgetDataInterface
{
	GENERATED_BODY()

public:
	virtual void SetWidgetData(UPrimaryDataAsset* Data) override;
};
