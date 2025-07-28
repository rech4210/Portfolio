

#include "UI/Enemy/BossStatusWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Shared/AI/EnemySystemCore/FEnemyWidgetData.h"


void UBossStatusWidget::SetWidget(const FEnemyWidgetData& WidgetData, const FBossDataStruct& Data){
	// 1) ?�볼 ?�팅
	if (Symbol_Image && WidgetData.Symbol)
	{
		Symbol_Image->SetBrushFromTexture(WidgetData.Symbol);
	}

	if (EnemyType_Text) {
		EnemyType_Text->SetText(FText::FromName(WidgetData.Type));
	}
	

	// 2) ?�름 ?�팅
	if (Name_Text)
	{
		Name_Text->SetText(FText::FromName(WidgetData.Name));
	}

	UpdateWidget(Data);

	// 4) ?�?�머 ?�정 (?�수�?무시)
	if (WidgetData.LimitTime > 0.f)
	{
		SetBossTimer(WidgetData.LimitTime);
	}
}


void UBossStatusWidget::UpdateWidget(const FBossDataStruct& Data) {
	UpdateHealthBar(Data);
}
void UBossStatusWidget::UpdateHealthBar(const FBossDataStruct& Data){
	const float CurrHP = Data.Health;
	const float MaxHP  = Data.MaxHealth;
	const float Ratio  = (MaxHP > 0.f) ? (CurrHP / MaxHP) : 0.f;

	
	if (HP_Bar)
	{
		HP_Bar->SetPercent(Ratio);
	}


	// if (HP_Text)
	// {
	// 	const FText Text = FText::Format(
	// 		NSLOCTEXT("Boss", "HPFormat", "{0} / {1}"),
	// 		FText::AsNumber(FMath::RoundToInt(CurrHP)),
	// 		FText::AsNumber(FMath::RoundToInt(MaxHP))
	// 	);
	// 	HP_Text->SetText(Text);
	// }

	if (HP_Text) {
		const FText Text = FText::Format(
			NSLOCTEXT("Boss", "HPFormat", "{0}"),
			FText::AsNumber(FMath::RoundToInt(CurrHP))
		);
		HP_Text->SetText(Text);
	}
}


void UBossStatusWidget::SetBossTimer(float DisplayTime) {
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_BossDisplay)) {
		return;
	}
	RemainingTime = DisplayTime;

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_BossDisplay,
		[this]()
		{
			RemainingTime = FMath::Max(0.f, RemainingTime - 1.f);

			int32 TotalSeconds = FMath::FloorToInt(RemainingTime);
			int32 Minutes = TotalSeconds / 60;
			int32 Seconds = TotalSeconds % 60;

			if (LimitTime_Text)
			{
				FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
				LimitTime_Text->SetText(FText::FromString(TimeString));
			}
		},
		1.0f, true
	);
}
