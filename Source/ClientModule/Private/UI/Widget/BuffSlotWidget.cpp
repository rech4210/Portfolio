// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/BuffSlotWidget.h"
#include "TimerManager.h"
#include "Components/Image.h"
#include "Engine/World.h"

void UBuffSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (RemainTimeBar){
		RemainTimeBar->SetPercent(1.f); // 버프 시간 초기화
	}
	
	if (RemainTimeText){
		RemainTimeText->SetText(FText::FromString(TEXT("")));
	}
}



void UBuffSlotWidget::ConsumeDuration(float CooldownTime)
{
	TotalCooldownTime = CooldownTime;
	ElapsedCooldownTime = 0.f;

	// 0.05초마다 UpdateDurationDuration 호출
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle, this, &UBuffSlotWidget::UpdateDuration, 0.05f, true
	);
}
void UBuffSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (BuffToolTipWidget->GetVisibility() != ESlateVisibility::Visible) {
		BuffToolTipWidget->SetVisibility(ESlateVisibility::Visible);
		BuffToolTipWidget->SetIsEnabled(false); // 입력 무시 (hover, focus 등)
		FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
		BuffToolTipWidget->SetPositionInViewport(MousePosition, false); // false == absolute position
	}
	UE_LOG(LogTemp, Warning, TEXT("TooltipWidget on %s"), *BuffToolTipWidget.GetName());
}

void UBuffSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseLeave(InMouseEvent);
	if (BuffToolTipWidget->GetVisibility() != ESlateVisibility::Collapsed) {
		BuffToolTipWidget->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("TooltipWidget off %s"), *UEnum::GetValueAsString(BuffToolTipWidget->GetVisibility()));
	}
}

void UBuffSlotWidget::SetWidgetData(UPrimaryDataAsset* Data) {
	UBuffDataAsset * BuffData = Cast<UBuffDataAsset>(Data);
	if (nullptr != BuffData->Tooltip) {
		BuffToolTipWidget = Cast<UBuffToolTip>(BuffData->Tooltip);
	}
	SkillImage->SetBrushFromTexture(BuffData->Image);
	ConsumeDuration(BuffData->Duration);
}

void UBuffSlotWidget::UpdateDuration()
{
	ElapsedCooldownTime += 0.05f;
	float Ratio = 1.f - (ElapsedCooldownTime / TotalCooldownTime);
	
	// 프로그레스 바 갱신
	if (RemainTimeBar)
	{
		RemainTimeBar->SetPercent(Ratio);
	}
	
	// 텍스트 갱신 (소수점 1자리까지 보여줌)
	if (RemainTimeText)
	{
		float RemainSeconds = TotalCooldownTime - ElapsedCooldownTime;
		RemainSeconds = FMath::Max(0.f, RemainSeconds); // 음수 방지
	
		FString TimeString = FString::Printf(TEXT("%.1f"), RemainSeconds);
		RemainTimeText->SetText(FText::FromString(TimeString));
	}
	
	// 쿨타임 완료 처리
	if (ElapsedCooldownTime >= TotalCooldownTime)
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
	
		if (RemainTimeBar)
		{
			RemainTimeBar->SetPercent(0.f);
		}
	
		if (RemainTimeText)
		{
			RemainTimeText->SetText(FText::FromString(TEXT("")));
		}
		BuffToolTipWidget->RemoveFromParent();
		RemoveFromParent();
	}
}
