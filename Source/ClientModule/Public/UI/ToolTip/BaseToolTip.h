// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseToolTip.generated.h"

class UPrimaryDataAsset;
class UTextBlock;
class UImage;

UCLASS()
class CLIENTMODULE_API UBaseToolTip : public UUserWidget {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(BindWidget),Category="ToolTip")
	TObjectPtr<UTextBlock> ToolTipTitle;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(BindWidget),Category="ToolTip")
	TObjectPtr<UTextBlock> ToolTipDescription;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(BindWidget), Category="ToolTip")
	TObjectPtr<UImage> ToolTipIcon;

	UFUNCTION()
	virtual void SetToolTipData(UPrimaryDataAsset* Data);
};
