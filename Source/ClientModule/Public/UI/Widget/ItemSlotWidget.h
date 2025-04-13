// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemSlotWidget.generated.h"

class UItemDataAsset;
/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	TArray<UItemSlotWidget*> ItemSlots;
	TObjectPtr<UItemDataAsset> ItemDataAsset;

private:
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
