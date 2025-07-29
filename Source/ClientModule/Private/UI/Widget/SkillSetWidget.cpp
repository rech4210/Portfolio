

#include "UI/Widget/SkillSetWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "UI/Widget/SkillSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "SkillModule/Public/Data/SkillInputSlot.h"
#include "SkillModule/Public/Utill/USkillHelper.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
// #include "Shared/Utill/USkillHelper.h"

#define MAX_SKILL_SLOT 8

void USkillSetWidget::InitWidget()
{
	SkillContainers.Add(SkillSlotContainer_1);
	SkillContainers.Add(SkillSlotContainer_2);

	SkillSlots.SetNum(MAX_SKILL_SLOT);

	for (int32 i = 0; i < MAX_SKILL_SLOT; ++i)
	{
		USkillSlotWidget* NewSlot = CreateWidget<USkillSlotWidget>(this, SkillSlotWidgetClass);
		SkillSlots[i] = NewSlot;

		int32 Row = i / 4;
		if (SkillContainers.IsValidIndex(Row))
		{
			SkillContainers[Row]->AddChild(NewSlot);
		}
	}
}

void USkillSetWidget::SetWidgetData(UPrimaryDataAsset* Data) {
	
	if (USkillDataAsset* SkillData = Cast<USkillDataAsset>(Data)) {
		SkillSlots[SkillData->SkillSlotIndex]->UseSkillSlot(SkillData);
		// SkillSlots[USkillHelper::GetValueFromKey(SkillData->SkillSlotIndex)]->ApplySkillData(SkillData);
	}
}

void USkillSetWidget::InitSkillWidgetData(UPrimaryDataAsset* Data) {
	if (USkillDataAsset* SkillData = Cast<USkillDataAsset>(Data)) {
		SkillSlots[SkillData->SkillSlotIndex]->ApplySkillData(SkillData);
	}
}


FReply USkillSetWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

bool USkillSetWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation) {
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
}

void USkillSetWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                           UDragDropOperation*& OutOperation) {
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	// DragOp->Payload = SkillDataAsset;
	DragOp->DefaultDragVisual = this;
	OutOperation = DragOp;
}
