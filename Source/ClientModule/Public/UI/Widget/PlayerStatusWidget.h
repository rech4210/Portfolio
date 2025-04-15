// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UI/SetWidgetDataInterface.h"
#include "PlayerStatusWidget.generated.h"

/**
 * 
 */
class UBuffDataAsset;
class UHorizontalBox;
class UBuffSlotWidget;
class UProgressBar;
class UTextBlock;
class UBuffToolTip;
UCLASS()
class CLIENTMODULE_API UPlayerStatusWidget : public UUserWidget, public ISetWidgetDataInterface
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UProgressBar> HealthBarProgress;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UProgressBar> ManaBarProgress;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock> HealthText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock> ManaText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Buff Widget")
	TObjectPtr<UHorizontalBox> BuffHorizontalBar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Buff Widget")
	TSubclassOf<UBuffSlotWidget> BuffSlotWidgetClass;

	void InitWidget();
	void UpdateHealthBar(float Current, float Max) const;
	void UpdateManaBar(float Current, float Max) const;
	virtual void SetWidgetData(UPrimaryDataAsset* Data) override;
};
