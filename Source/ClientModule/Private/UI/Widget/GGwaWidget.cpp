// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GGwaWidget.h"
#include "AbilitySystemComponent.h"
#include "UI/Widget/PlayerStatusWidget.h"
#include "UI/Widget/ItemSetWidget.h"
#include "UI/Widget/SkillSetWidget.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/ItemDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"

#include "UI/ToolTip/BuffToolTip.h"
#include "UI/ToolTip/ItemToolTip.h"
#include "UI/ToolTip/SkillToolTip.h"

void UGGwaWidget::NativeConstruct() {
	
}

void UGGwaWidget::SetWidgetData(UPrimaryDataAsset* Data) {
	// 데이터 설정 시점에서 툴팁도 만들기, skill, item 등은 begin play에서 툴팁 만들어주는것이 좋음.
	// Data driven 방식에서 Widget을 Buff에 넣어 관리하는건 안좋을 수 있다. 수정해야할지도.
	if (UBuffDataAsset * BuffData = Cast<UBuffDataAsset>(Data)) {
		UBuffToolTip* BuffToolTipWidget = CreateWidget<UBuffToolTip>(this, BuffToolTipClass);
		BuffToolTipWidget->SetToolTipData(BuffData);
		BuffData->Tooltip = BuffToolTipWidget;
		BP_PlayerStatusWidget->SetWidgetData(BuffData);
	}
	else if (USkillDataAsset * SkillData = Cast<USkillDataAsset>(Data)) {
		USkillToolTip* SkillToolTipWidget = CreateWidget<USkillToolTip>(this, SkillToolTipClass);
		SkillToolTipWidget->SetToolTipData(SkillData);
		SkillData->Tooltip = SkillToolTipWidget;
		BP_SkillBarWidget->SetWidgetData(SkillData);
	}
	else if (UItemDataAsset * ItemData = Cast<UItemDataAsset>(Data)) {
		UItemToolTip* ItemToolTipWidget = CreateWidget<UItemToolTip>(this, ItemToolTipClass);
		ItemToolTipWidget->SetToolTipData(ItemData);
		ItemData->Tooltip = ItemToolTipWidget;
		BP_ItemBarWidget->SetWidgetData(ItemData);
	}
}

void UGGwaWidget::InitWidget(UAbilitySystemComponent* AbilitySystemComponent, const UGGwaAttributeSet* AttributeSet) {
	if (!AbilitySystemComponent || !AttributeSet) return;
	ASC = AbilitySystemComponent;
	GGwaAttributeSet = AttributeSet;

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

