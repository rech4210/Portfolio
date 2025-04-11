// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemComponent.h"
#include "UI/Widget/GGwaWidget.h"
#include "UI/Widget/PlayerStatusWidget.h"
#include "UI/Widget/ItemSetWidget.h"
#include "UI/Widget/SkillSetWidget.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/ItemDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"

void UGGwaWidget::NativeConstruct() {
	
}

void UGGwaWidget::SetWidgetData(UPrimaryDataAsset* Data) {
	if (UBuffDataAsset * BuffData = Cast<UBuffDataAsset>(Data)) {
		BP_PlayerStatusWidget->SetWidgetData(BuffData);
	}
	else if (USkillDataAsset * SkillData = Cast<USkillDataAsset>(Data)) {
		BP_SkillBarWidget->SetWidgetData(SkillData);
	}
	else if (UItemDataAsset * ItemData = Cast<UItemDataAsset>(Data)) {
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

