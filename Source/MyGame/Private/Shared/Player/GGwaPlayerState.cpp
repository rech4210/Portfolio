// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "AbilitySystemComponent.h"


AGGwaPlayerState::AGGwaPlayerState() {
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UGGwaAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* AGGwaPlayerState::GetAbilitySystemComponent() const {
	return ASC.Get() ? ASC.Get() : nullptr;
}
