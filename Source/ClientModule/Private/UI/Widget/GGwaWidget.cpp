// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GGwaWidget.h"
#include "AbilitySystemComponent.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
#include "UI/Widget/PlayerStatusWidget.h"
#include "UI/Widget/ItemSetWidget.h"
#include "UI/Widget/SkillSetWidget.h"

#include "GameSharedModule/Public/Data/BuffDataAsset.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "GameSharedModule/Public/Data/BaseDataAsset.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"

#include "UI/ToolTip/BuffToolTip.h"
#include "UI/ToolTip/ItemToolTip.h"
#include "UI/ToolTip/SkillToolTip.h"
#include "UI/Widget/BuffSlotWidget.h"


// TMap<EPlayerState, TFunction<void()>> StateHandlers;
//
// void InitializeStateHandlers()
// {
// 	StateHandlers.Add(EPlayerState::Stunned, [this]() { ShowStunUI(); });
// 	StateHandlers.Add(EPlayerState::Dead, [this]() { ShowDeathScreen(); });
// }


// buff의 경우 재생성 하므로 우선 구현. 나중에 Pooling 적용후 삭제 예정
// 사용과 widget 바인딩을 분리할것.
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


void UGGwaWidget::InitWidget(UGGwaAbilitySystemComponent* AbilitySystemComponent, const UGGwaAttributeSet* AttributeSet) {
	if (!AbilitySystemComponent || !AttributeSet) return;

	ASC = AbilitySystemComponent;
	GGwaAttributeSet = AttributeSet;
	
	// Bind to the new delegate from the ASC for buff/skill/item UI updates
	ASC->OnEffectAssetApplied.AddDynamic(this, &UGGwaWidget::BindWidgetWithTooltip);
	
	BP_PlayerStatusWidget->InitWidget();
	BP_SkillBarWidget->InitWidget();
	BP_ItemBarWidget->InitWidget();

	BP_PlayerStatusWidget->UpdateHealthBar(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	BP_PlayerStatusWidget->UpdateManaBar(AttributeSet->GetMana(), AttributeSet->GetMaxMana());

	for (auto& SkillData : InitSkillDataAssets) {
		BindWidgetWithTooltip(SkillData);
	}
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
