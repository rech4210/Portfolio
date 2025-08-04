
#include "UI/Widget/BuffSlotWidget.h"
#include "TimerManager.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UBuffSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (RemainTimeBar){
		RemainTimeBar->SetPercent(1.f);
	}
	
	if (RemainTimeText){
		RemainTimeText->SetText(FText::FromString(TEXT("")));
	}
}



void UBuffSlotWidget::ConsumeDuration(float CooldownTime)
{
	TotalCooldownTime = CooldownTime;
	ElapsedCooldownTime = 0.f;

	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle, this, &UBuffSlotWidget::UpdateDuration, 0.05f, true
	);
}
void UBuffSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (BuffToolTipWidget->GetVisibility() != ESlateVisibility::Visible) {
		BuffToolTipWidget->SetVisibility(ESlateVisibility::Visible);
		BuffToolTipWidget->SetIsEnabled(false);
		FVector2D LocalMousePos;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(LocalMousePos.X, LocalMousePos.Y);
		BuffToolTipWidget->SetPositionInViewport(LocalMousePos, true);
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
	
	if (RemainTimeBar)
	{
		RemainTimeBar->SetPercent(Ratio);
	}
	
	if (RemainTimeText)
	{
		float RemainSeconds = TotalCooldownTime - ElapsedCooldownTime;
		RemainSeconds = FMath::Max(0.f, RemainSeconds); 
	
		FString TimeString = FString::Printf(TEXT("%.1f"), RemainSeconds);
		RemainTimeText->SetText(FText::FromString(TimeString));
	}
	
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

