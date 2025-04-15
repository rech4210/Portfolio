// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GGwaWidget.h"
#include "AbilitySystemComponent.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
#include "UI/Widget/PlayerStatusWidget.h"
#include "UI/Widget/ItemSetWidget.h"
#include "UI/Widget/SkillSetWidget.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/ItemDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/Data/BaseDataAsset.h"

#include "UI/ToolTip/BuffToolTip.h"
#include "UI/ToolTip/ItemToolTip.h"
#include "UI/ToolTip/SkillToolTip.h"
#include "UI/Widget/BuffSlotWidget.h"

void UGGwaWidget::NativeConstruct() {
	
}


void UGGwaWidget::BindWidgetWithTooltip(UBaseDataAsset* Data) {
	if (auto* ValueWidget = SlotTooltipCachePair.FindRef(Data); ValueWidget!= nullptr) {
		// buff의 경우 재생성 하므로 우선 구현. 나중에 Pooling 적용후 삭제 예정
		if (Cast<UBuffToolTip>(ValueWidget)) {
			BP_PlayerStatusWidget->SetWidgetData(Data);
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
		BP_SkillBarWidget->SetWidgetData(BuffData);
		SlotTooltipCachePair.Add(SkillData, NewToolTip);
	}
	else if (UItemDataAsset* ItemData = Cast<UItemDataAsset>(Data))
	{
		UItemToolTip* NewToolTip = CreateWidget<UItemToolTip>(this, ItemToolTipClass);
		NewToolTip->SetToolTipData(ItemData);
		ItemData->Tooltip = NewToolTip;
		BP_ItemBarWidget->SetWidgetData(BuffData);
		SlotTooltipCachePair.Add(ItemData, NewToolTip);
	}
}

void UGGwaWidget::InitWidget(UAbilitySystemComponent* AbilitySystemComponent, const UGGwaAttributeSet* AttributeSet) {
	if (!AbilitySystemComponent || !AttributeSet) return;
	ASC = AbilitySystemComponent;
	GGwaAttributeSet = AttributeSet;
	
	BP_PlayerStatusWidget->InitWidget();
	BP_SkillBarWidget->InitWidget();
	BP_ItemBarWidget->InitWidget();

	// 초기 값 표시
	BP_PlayerStatusWidget->UpdateHealthBar(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	BP_PlayerStatusWidget->UpdateManaBar(AttributeSet->GetMana(), AttributeSet->GetMaxMana());


	// 델리게이트 바인딩
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


// template<typename TDataAsset, typename TToolTip, typename TWidget>
// void UGGwaWidget::BindTooltip(TDataAsset* DataAsset, TSubclassOf<TToolTip> ToolTipClass, UUserWidget* ContainerWidget)
// {
// 	if (!DataAsset || !ToolTipClass || !ContainerWidget) return;
//
// 	TToolTip* NewToolTip = CreateWidget<TToolTip>(this, ToolTipClass);
// 	NewToolTip->SetToolTipData(DataAsset);
// 	DataAsset->Tooltip = NewToolTip;
//
// 	TWidget* SlotWidget = Cast<TWidget>(ISetWidgetDataInterface::SetWidgetData(DataAsset));
// 	SlotTooltipCachePair.Add(SlotWidget, NewToolTip);
// }
//
