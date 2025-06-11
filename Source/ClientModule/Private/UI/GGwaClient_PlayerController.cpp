// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GGwaClient_PlayerController.h"
#include "UI/Widget/GGwaWidget.h"
#include "UI/GGwaHUD.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "UI/Enemy/BossStatusWidget.h"
#include "Shared/Player/GGwaPlayerState.h"


void AGGwaClient_PlayerController::BeginPlay() {
	Super::BeginPlay();

}

void AGGwaClient_PlayerController::InitClientWidget() {
	if (HasAuthority()) {
		UE_LOG(LogTemp, Warning, TEXT("server Controller"));
		return;
	}
	else if (IsLocalController()) {
		UE_LOG(LogTemp, Warning, TEXT("Client has been initialized"));
	}
	if (WidgetClass) {
		UGGwaWidget * Widget = CreateWidget<UGGwaWidget>(this, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(this, BossStatusWidgetClass);
		Widget->AddToViewport();
		BossWidget->AddToViewport();
		if (Widget) {
			GGwaHUD = Cast<AGGwaHUD>(GetHUD());
			GGwaHUD->SetBaseWidget(Widget);
			
			if (AGGwaPlayerState * PS = GetPlayerState<AGGwaPlayerState>()) {
				auto ASC = PS->GetAbilitySystemComponent();
				UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
				const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				Widget->InitWidget(GGawASC, GGwaAttributeSet);
			}
		}
	}
}

void AGGwaClient_PlayerController::Client_ApplyAbilityDataAsset_Implementation(UBaseDataAsset* Data) {
	if (GGwaHUD && IsLocalController()) {
		// Is it call twice?
		GGwaHUD->GetBaseWidget()->BindWidgetWithTooltip(Data);
	}
}

void AGGwaClient_PlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& Data) {
	if (GGwaHUD && IsLocalController()) {
		GGwaHUD->GetBossWidget()->UpdateBossWidget(Data);
	}
}


// void AGGwaClient_PlayerController::Client_ApplyAbilityDataAsset(UBaseDataAsset* Data) {
// 	if (GGwaHUD && IsLocalController()) {
// 		GGwaHUD->GetBaseWidget()->BindWidgetWithTooltip(Data);
// 	}
// }
//
// void AGGwaClient_PlayerController::Client_ReceiveBossData(const FBossDataStruct& Data) {
// 	if (GGwaHUD && IsLocalController()) {
// 		GGwaHUD->GetBossWidget()->UpdateBossWidget(Data);
// 	}
// }

