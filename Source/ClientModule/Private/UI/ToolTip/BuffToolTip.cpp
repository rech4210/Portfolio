// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ToolTip/BuffToolTip.h"

#include "Components/Image.h"
#include "Shared/Data/BuffDataAsset.h"
#include "Components/TextBlock.h"


void UBuffToolTip::SetToolTipData(UPrimaryDataAsset* ToolTip) {
	Super::SetToolTipData(ToolTip);
	if (UBuffDataAsset* BuffData = Cast<UBuffDataAsset>(ToolTip)) {
		if (ToolTipTitle) {
			ToolTipTitle->SetText(BuffData->DisplayName);
		}
		if (ToolTipDescription) {
			ToolTipDescription->SetText(BuffData->Description); // TODO: 실제 설명으로 변경
		}
		if (ToolTipIcon) {
			ToolTipIcon->SetBrushFromTexture(BuffData->Image);
		}
		// SetVisibility(ESlateVisibility::Visible);
	}
}
