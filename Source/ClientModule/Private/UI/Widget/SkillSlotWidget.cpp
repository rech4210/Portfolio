// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/SkillSlotWidget.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Shared/GAS/SkillInputSlot.h"
#include "Shared/Utill/USkillHelper.h"
#include "UI/ToolTip/SkillToolTip.h"

// USkillSlotWidget::USkillSlotWidget() {
//
// }

FReply USkillSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USkillSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                            UDragDropOperation*& OutOperation) {
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}


void USkillSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (SkillToolTipWidget->GetVisibility() != ESlateVisibility::Visible) {
		SkillToolTipWidget->SetVisibility(ESlateVisibility::Visible);
		SkillToolTipWidget->SetIsEnabled(false); // 입력 무시 (hover, focus 등)
		FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
		SkillToolTipWidget->SetPositionInViewport(MousePosition, false); // false == absolute position
	}
	UE_LOG(LogTemp, Warning, TEXT("TooltipWidget on %s"), *SkillToolTipWidget.GetName());
}

void USkillSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseLeave(InMouseEvent);
	if (SkillToolTipWidget->GetVisibility() != ESlateVisibility::Collapsed) {
		SkillToolTipWidget->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("TooltipWidget off %s"), *UEnum::GetValueAsString(SkillToolTipWidget->GetVisibility()));
	}
}
void USkillSlotWidget::UseSkillSlot(USkillDataAsset* Data) {
	//생성자에서 기본적인 스킬 데이터 초기화 -> 다른 데이터 들어올 경우, 해당 데이터로 초기화
	if (SkillDataAsset != Data) {
		ApplySkillData(Data);
	}
	UE_LOG(LogTemp, Warning, TEXT("USkillSlotWidget::UseSkillSlot .%s"), *this->GetName());

	// 쿨타임 체크. 다만 GAS 에서 GA를 호출하기 때문에 아마 발동 안되게 할것임.
	CurrentCoolTime = 0.f;

	if (CooldownImage)
	{
		CooldownImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (CoolTimeText)
	{
		CoolTimeText->SetVisibility(ESlateVisibility::Visible);
	}
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &USkillSlotWidget::TickCoolDown, 0.05f, true);
	TickCoolDown();
}

void USkillSlotWidget::NativeConstruct() {
	SetVisibility(ESlateVisibility::Hidden);
	if (CooldownImage && !SkillCooldownMaterial) {
		if (UMaterialInterface* BaseMat = Cast<UMaterialInterface>(CooldownImage->GetBrush().GetResourceObject())) {
			SkillCooldownMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
			CooldownImage->SetBrushFromMaterial(SkillCooldownMaterial);
		}
	}
}

void USkillSlotWidget::ApplySkillData(USkillDataAsset* NewData) {
	if (NewData == nullptr) {
		UE_LOG(LogTemp,Warning,TEXT("USkillSlotWidget::ApplySkillData: nullptr"));
		return;
	}
	
	SetVisibility(ESlateVisibility::Visible);

	SkillDataAsset = NewData;
	CoolTime = NewData->CoolTime;
	SkillImage->SetBrushFromTexture(NewData->Image);
	SkillKeyText->SetText(FText::FromString(USkillHelper::GetKeyString(NewData->SkillSlotIndex)));
	SkillToolTipWidget = Cast<USkillToolTip>(NewData->Tooltip);
	// SkillWidgetToolTip =
}

void USkillSlotWidget::TickCoolDown() {
	CurrentCoolTime += 0.05f;
	float Percent = FMath::Clamp(CurrentCoolTime / CoolTime, 0.f, 1.f);
	SkillCooldownMaterial->SetScalarParameterValue(FName("Progress"), Percent);

	// 2. 쿨타임 텍스트 갱신
	if (CoolTimeText)
	{
		float RemainingTime = FMath::Max(0.f, CoolTime - CurrentCoolTime);
		CoolTimeText->SetText(FText::FromString(FString::Printf(TEXT("%ds"), FMath::CeilToInt(RemainingTime))));
	}

	// 3. 쿨타임 종료 처리
	if (Percent >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
		CurrentCoolTime = 0.f;

		// 이미지, 텍스트 숨기기 등 처리
		if (CooldownImage)
		{
			CooldownImage->SetVisibility(ESlateVisibility::Hidden);
		}
		if (CoolTimeText)
		{
			CoolTimeText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
