// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/ItemSlotWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Shared/Data/ItemDataAsset.h"

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation) {
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                           UDragDropOperation*& OutOperation) {
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	DragOp->Payload = ItemDataAsset;
	DragOp->DefaultDragVisual = this;
	OutOperation = DragOp;
}
