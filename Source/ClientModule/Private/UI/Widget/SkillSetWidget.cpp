// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/SkillSetWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Shared/Data/SkillDataAsset.h"
#include "UI/Widget/SkillSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Shared/GAS/SkillInputSlot.h"
#include "Shared/Utill/USkillHelper.h"
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

		// ✅ 줄 기준으로 컨테이너 결정 (4개씩 나눔)
		int32 Row = i / 4;
		if (SkillContainers.IsValidIndex(Row))
		{
			SkillContainers[Row]->AddChild(NewSlot);
		}
	}
}

void USkillSetWidget::SetWidgetData(UPrimaryDataAsset* Data) {
	
	// 현재 이 부분은 Skill Use와 연관되어 있음. 스킬 초기화는 따로 해주는게 맞다.
	SkillSlots[USkillHelper::GetValueFromKey(SkillDataAsset->SkillSlotIndex)]->UseSkillSlot(Cast<USkillDataAsset>(SkillDataAsset));
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
	DragOp->Payload = SkillDataAsset;
	DragOp->DefaultDragVisual = this;
	OutOperation = DragOp;
}
