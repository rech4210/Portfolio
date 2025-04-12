// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ToolTip/BaseToolTip.h"

void UBaseToolTip::SetToolTipData(UPrimaryDataAsset* Data) {
	if (/*already exsits*/false) {
		return;
	}
	AddToViewport(999);
	SetVisibility(ESlateVisibility::Collapsed);
}
