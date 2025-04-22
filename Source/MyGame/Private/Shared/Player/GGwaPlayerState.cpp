// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"


AGGwaPlayerState::AGGwaPlayerState() {
	ASC = CreateDefaultSubobject<UGGwaAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UGGwaAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* AGGwaPlayerState::GetAbilitySystemComponent() const {
	return ASC.Get() ? ASC.Get() : nullptr;
}
