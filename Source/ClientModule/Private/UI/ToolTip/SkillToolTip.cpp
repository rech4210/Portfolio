

#include "UI/ToolTip/SkillToolTip.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"

void USkillToolTip::SetToolTipData(UPrimaryDataAsset* Data) {
	// Super::SetToolTipData(ToolTip);
	AddToViewport(999);
	SetVisibility(ESlateVisibility::Collapsed);
	if (USkillDataAsset* BuffData = Cast<USkillDataAsset>(Data)) {
		if (ToolTipTitle) {
			ToolTipTitle->SetText(BuffData->DisplayName);
		}
		if (ToolTipDescription) {
			ToolTipDescription->SetText(BuffData->Description); // TODO: ���� �������� ����
		}
		if (ToolTipIcon) {
			ToolTipIcon->SetBrushFromTexture(BuffData->Image);
		}
		// SetVisibility(ESlateVisibility::Visible);
	}
}
