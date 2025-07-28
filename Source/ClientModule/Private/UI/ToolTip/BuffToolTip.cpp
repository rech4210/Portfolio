

#include "UI/ToolTip/BuffToolTip.h"

#include "Components/Image.h"
#include "GameSharedModule/Public/Data/BuffDataAsset.h"
#include "Components/TextBlock.h"


void UBuffToolTip::SetToolTipData(UPrimaryDataAsset* Data) {
	// Super::SetToolTipData(ToolTip);
	AddToViewport(999);
	SetVisibility(ESlateVisibility::Collapsed);
	if (const UBuffDataAsset* BuffData = Cast<UBuffDataAsset>(Data)) {
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
