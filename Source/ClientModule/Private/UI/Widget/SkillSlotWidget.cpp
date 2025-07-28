// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/SkillSlotWidget.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "SkillModule/Public/Data/SkillInputSlot.h"
#include "SkillModule/Public/Utill/USkillHelper.h"
#include "UI/ToolTip/SkillToolTip.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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
		SkillToolTipWidget->SetIsEnabled(false); // ?…ë ¥ ë¬´ì‹œ (hover, focus ??
		FVector2D LocalMousePos;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(LocalMousePos.X, LocalMousePos.Y);
		SkillToolTipWidget->SetPositionInViewport(LocalMousePos, true); // DPI ë³€??ê³ ë ¤??
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
	//?ì„±?ì—??ê¸°ë³¸?ì¸ ?¤í‚¬ ?°ì´??ì´ˆê¸°??-> ?¤ë¥¸ ?°ì´???¤ì–´??ê²½ìš°, ?´ë‹¹ ?°ì´?°ë¡œ ì´ˆê¸°??
	if (SkillDataAsset != Data) {
		ApplySkillData(Data);
	}
	UE_LOG(LogTemp, Warning, TEXT("USkillSlotWidget::UseSkillSlot .%s"), *this->GetName());
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
	CooldownImage->SetVisibility(ESlateVisibility::Hidden);
	CoolTimeText->SetVisibility(ESlateVisibility::Hidden);
	// SkillWidgetToolTip =
}

// void USkillSlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
// 	UDragDropOperation* InOperation) {
// 	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
// 	
// }
//
// void USkillSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) {
// 	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
// 	
// }
//
// void USkillSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) {
// 	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
// 	
// }

void USkillSlotWidget::TickCoolDown() {
	CurrentCoolTime += 0.05f;
	float Percent = FMath::Clamp(CurrentCoolTime / CoolTime, 0.f, 1.f);
	SkillCooldownMaterial->SetScalarParameterValue(FName("Progress"), Percent);

	// 2. ì¿¨í????ìŠ¤??ê°±ì‹ 
	if (CoolTimeText)
	{
		float RemainingTime = FMath::Max(0.f, CoolTime - CurrentCoolTime);
		CoolTimeText->SetText(FText::FromString(FString::Printf(TEXT("%ds"), FMath::CeilToInt(RemainingTime))));
	}

	// 3. ì¿¨í???ì¢…ë£Œ ì²˜ë¦¬
	if (Percent >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
		CurrentCoolTime = 0.f;

		// ?´ë?ì§€, ?ìŠ¤???¨ê¸°ê¸???ì²˜ë¦¬
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
