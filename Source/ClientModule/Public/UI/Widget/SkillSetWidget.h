// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SetWidgetDataInterface.h"
#include "SkillSetWidget.generated.h"

class UHorizontalBox;
class USkillSlotWidget;
class USkillDataAsset;
/**
 * 
 */
UCLASS()
class CLIENTMODULE_API USkillSetWidget : public UUserWidget, public ISetWidgetDataInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillSetWidget")
	TSubclassOf<USkillSlotWidget> SkillSlotWidgetClass;
	
	UPROPERTY(VisibleAnywhere, Category = "Slot")
	TArray<USkillSlotWidget*> SkillSlots;
	UPROPERTY(VisibleAnywhere, Category = "Slot")
	TArray<UHorizontalBox*> SkillContainers;
	UPROPERTY(VisibleAnywhere,meta=(BindWidget) , Category = "Slot")
	TObjectPtr<UHorizontalBox> SkillSlotContainer_1;
	UPROPERTY(VisibleAnywhere,meta=(BindWidget) , Category = "Slot")
	TObjectPtr<UHorizontalBox> SkillSlotContainer_2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset; 

	void InitWidget();
	virtual void SetWidgetData(UPrimaryDataAsset* Data) override;
private:
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
