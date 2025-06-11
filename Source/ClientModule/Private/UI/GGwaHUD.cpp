// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GGwaHUD.h"
#include "UI/Enemy/BossStatusWidget.h"

UGGwaWidget* AGGwaHUD::GetBaseWidget() const { return BaseWidget; }

UBossStatusWidget* AGGwaHUD::GetBossWidget() const {
	return BossWidget;
}

void AGGwaHUD::SetBaseWidget(UGGwaWidget* widget){
	BaseWidget = widget;
}

void AGGwaHUD::SetBossWidget(UBossStatusWidget* widget) {
	BossWidget = widget;
}
