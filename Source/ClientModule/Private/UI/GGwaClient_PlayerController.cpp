// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GGwaClient_PlayerController.h"
#include "UI/Widget/GGwaWidget.h"
#include "UI/GGwaHUD.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/Data/BaseDataAsset.h"
#include "Shared/Player/GGwaPlayerState.h"


void AGGwaClient_PlayerController::BeginPlay() {
	Super::BeginPlay();
}

void AGGwaClient_PlayerController::InitClientWidget() {
	if (IsLocalController() && WidgetClass) {
		// Widget = CreateWidget(this, WidgetClass);
		UGGwaWidget * Widget = CreateWidget<UGGwaWidget>(this, WidgetClass);
		Widget->AddToViewport();
		if (Widget) {
			GGwaHUD = Cast<AGGwaHUD>(GetHUD());
			GGwaHUD->SetBaseWidget(Widget);
			UAbilitySystemComponent* ASC = GetPlayerState<AGGwaPlayerState>()->GetAbilitySystemComponent();
			const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(ASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
			Widget->InitWidget(ASC, GGwaAttributeSet);
		}
	}
}

void AGGwaClient_PlayerController::GetDataFromAbility(UBaseDataAsset* Data) {
	GGwaHUD->GetBaseWidget()->BindWidgetWithTooltip(Data);
}

