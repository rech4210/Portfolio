// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GGwaHUD.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "UI/Enemy/BossStatusWidget.h"
#include "GameSharedModule/Public/Data/BaseDataAsset.h"

void AGGwaHUD::HandleAbilityDataApplied(UBaseDataAsset* Data)
{
	if (BaseWidget)
	{
		BaseWidget->BindWidgetWithTooltip(Data);
	}
}

void AGGwaHUD::HandleBossDataReceived(const FBossDataStruct& BossData)
{
	if (BossWidget)
	{
		BossWidget->UpdateWidget(BossData);
	}
}

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
