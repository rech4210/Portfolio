// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GGwaHUD.h"

UGGwaWidget* AGGwaHUD::GetBaseWidget() const { return BaseWidget; }

void AGGwaHUD::SetBaseWidget(UGGwaWidget* widget){
	BaseWidget = widget;
}
