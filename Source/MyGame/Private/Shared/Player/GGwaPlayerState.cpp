// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/Component/PlayerReactionComponent.h"
#include "Shared/Player/Component/UPlayerStateComponent.h"
#include "Shared/Utill/FObservedAttributeHelper.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"


void AGGwaPlayerState::BeginPlay() {
	Super::BeginPlay();
	Character = Cast<AGGwaCharacter>(GetOwner());
	UEnumTagMatchHelper::InitializeHelper();
}


AGGwaPlayerState::AGGwaPlayerState() {
	ASC = CreateDefaultSubobject<UGGwaAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UGGwaAttributeSet>("AttributeSet");
	StateComponent = CreateDefaultSubobject<UPlayerStateComponent>("PlayerStateComponent");
}

UAbilitySystemComponent* AGGwaPlayerState::GetAbilitySystemComponent() const {
	return ASC.Get() ? ASC.Get() : nullptr;
}

UPlayerStateComponent* AGGwaPlayerState::GetStateComponent() const {
	return StateComponent.Get();
}

void AGGwaPlayerState::BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue, USkillDataAsset* SkillData) const{
	OnAttributeChanged.Broadcast(Attribute, NewValue, SkillData);
	// value 판정을 helper로
	// FObservedAttributeHelper<> Player의 status enum 필요
	if (Attribute == AttributeSet->GetHealthAttribute()) {
		if (NewValue <= 0.0f) {
			SetPlayerDeathState();
		}
	}
}

//server? client?
void AGGwaPlayerState::SetPlayerDeathState() const{
	StateComponent->PlayerDeath();
	Character->GetReactionComponent()->ExecuteDeadReaction();
}
