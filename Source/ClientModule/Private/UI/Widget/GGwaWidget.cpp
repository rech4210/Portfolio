

#include "UI/Widget/GGwaWidget.h"
#include "AbilitySystemComponent.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
#include "Entities/SkillSlot.h"
#include "UI/Widget/PlayerStatusWidget.h"
#include "UI/Widget/ItemSetWidget.h"
#include "UI/Widget/SkillSetWidget.h"
#include "SkillModule/Public/Components/SkillComponent.h"

#include "GameSharedModule/Public/Data/BuffDataAsset.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "GameSharedModule/Public/Data/BaseDataAsset.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"

#include "UI/ToolTip/BuffToolTip.h"
#include "UI/ToolTip/ItemToolTip.h"
#include "UI/ToolTip/SkillToolTip.h"
#include "UI/Widget/BuffSlotWidget.h"



void UGGwaWidget::BindWidgetWithTooltip(UBaseDataAsset* Data) {
	if (auto* ValueWidget = SlotTooltipCachePair.FindRef(Data); ValueWidget!= nullptr) {
		if (Cast<UBuffToolTip>(ValueWidget)) {
			BP_PlayerStatusWidget->SetWidgetData(Data);
		}
		else if (Cast<USkillToolTip>(ValueWidget)) {
			BP_SkillBarWidget->SetWidgetData(Data);
		}
		else if (Cast<UItemToolTip>(ValueWidget)) {
			BP_ItemBarWidget->SetWidgetData(Data);
		}
		return;
	}
	if (UBuffDataAsset* BuffData = Cast<UBuffDataAsset>(Data))
	{
		UBuffToolTip* NewToolTip = CreateWidget<UBuffToolTip>(this, BuffToolTipClass);
		NewToolTip->SetToolTipData(BuffData);
		BuffData->Tooltip = NewToolTip;
		BP_PlayerStatusWidget->SetWidgetData(BuffData);
		SlotTooltipCachePair.Add(BuffData, NewToolTip);
	}
	else if (USkillDataAsset* SkillData = Cast<USkillDataAsset>(Data))
	{
		USkillToolTip* NewToolTip = CreateWidget<USkillToolTip>(this, SkillToolTipClass);
		NewToolTip->SetToolTipData(SkillData);
		SkillData->Tooltip = NewToolTip;
		BP_SkillBarWidget->InitSkillWidgetData(SkillData);
		SlotTooltipCachePair.Add(SkillData, NewToolTip);
	}
	else if (UItemDataAsset* ItemData = Cast<UItemDataAsset>(Data))
	{
		UItemToolTip* NewToolTip = CreateWidget<UItemToolTip>(this, ItemToolTipClass);
		NewToolTip->SetToolTipData(ItemData);
		ItemData->Tooltip = NewToolTip;
		BP_ItemBarWidget->SetWidgetData(ItemData);
		SlotTooltipCachePair.Add(ItemData, NewToolTip);
	}
}


void UGGwaWidget::DoWidgetWork() {
}


void UGGwaWidget::UpdateSkillWidgetFromServer(TArray<USkillDataAsset*> Datas) {
	for (UBaseDataAsset* Data : Datas) {
		BindWidgetWithTooltip(Data);
	}
}


void UGGwaWidget::InitWidget(UGGwaAbilitySystemComponent* AbilitySystemComponent, const UGGwaAttributeSet* AttributeSet) {
	if (!AbilitySystemComponent || !AttributeSet) return;

	ASC = AbilitySystemComponent;
	GGwaAttributeSet = AttributeSet;
	
	ASC->OnEffectAssetApplied.AddDynamic(this, &UGGwaWidget::BindWidgetWithTooltip);
	
	BP_PlayerStatusWidget->InitWidget();
	BP_SkillBarWidget->InitWidget();
	BP_ItemBarWidget->InitWidget();

	BP_PlayerStatusWidget->UpdateHealthBar(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	BP_PlayerStatusWidget->UpdateManaBar(AttributeSet->GetMana(), AttributeSet->GetMaxMana());

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
		.AddUObject(this, &UGGwaWidget::OnHealthChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetManaAttribute())
		.AddUObject(this, &UGGwaWidget::OnManaChanged);
}


void UGGwaWidget::OnHealthChanged(const FOnAttributeChangeData& Data) const{
	BP_PlayerStatusWidget->UpdateHealthBar(Data.NewValue, GGwaAttributeSet->GetMaxHealth());
}

void UGGwaWidget::OnManaChanged(const FOnAttributeChangeData& Data) const{
	BP_PlayerStatusWidget->UpdateManaBar(Data.NewValue, GGwaAttributeSet->GetMaxMana());
}
