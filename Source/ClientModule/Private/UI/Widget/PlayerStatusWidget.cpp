// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/PlayerStatusWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "UI/Widget/BuffSlotWidget.h"


void UPlayerStatusWidget::UpdateHealthBar(float Current, float Max) const{
	if (HealthBarProgress)
	{
		HealthBarProgress->SetPercent(Current / Max);
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::FloorToInt(Current), FMath::FloorToInt(Max))));

	}
}

void UPlayerStatusWidget::UpdateManaBar(float Current, float Max) const{
	if (ManaBarProgress)
	{
		ManaBarProgress->SetPercent(Current / Max);
		ManaText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::FloorToInt(Current), FMath::FloorToInt(Max))));
	}
}

void UPlayerStatusWidget::AddBuff(UPrimaryDataAsset* Data) {
	UBuffSlotWidget* Widget = CreateWidget<UBuffSlotWidget>(this, BuffSlotWidgetClass);
	Widget->SetWidgetData(Data);
	BuffHorizontalBar->AddChildToHorizontalBox(Widget);
}

void UPlayerStatusWidget::SetWidgetData(UPrimaryDataAsset* Data) {
	UE_LOG(LogTemp, Warning, TEXT("PlayerStatusWidget SetWidgetData"));
	AddBuff(Data);
}
