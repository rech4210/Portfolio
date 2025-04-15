// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ToolTip/SkillToolTip.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Shared/Data/SkillDataAsset.h"

void USkillToolTip::SetToolTipData(UPrimaryDataAsset* Data) {
	// Super::SetToolTipData(ToolTip);
	AddToViewport(999);
	SetVisibility(ESlateVisibility::Collapsed);
	if (USkillDataAsset* BuffData = Cast<USkillDataAsset>(Data)) {
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
