// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GGwaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UGGwaAttributeSet::UGGwaAttributeSet() {
	InitHealth(100.f);
	InitMana(70.f);
	InitMaxHealth(200.f);
	InitMaxMana(100.f);
}

void UGGwaAttributeSet::OnRep_Health(const FGameplayAttributeData& Old) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGwaAttributeSet, Health, Old);
}

void UGGwaAttributeSet::OnRep_Mana(const FGameplayAttributeData& Old) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGwaAttributeSet, Mana, Old);
}

void UGGwaAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& Old) const{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGwaAttributeSet, MaxHealth, Old);
}

void UGGwaAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& Old) const{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGGwaAttributeSet, MaxMana, Old);
}

void UGGwaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) {
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute()) {
		NewValue = FMath::Clamp(NewValue,0.f,GetMaxHealth());
	}
}

void UGGwaAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) {
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute()) {
		SetHealth(FMath::Clamp(GetHealth(),0.f,GetMaxHealth()));
		
	}
}


void UGGwaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

