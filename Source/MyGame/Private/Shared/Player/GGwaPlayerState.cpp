// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerState.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/Component/UPlayerStateComponent.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"




AGGwaPlayerState::AGGwaPlayerState() {
	ASC = CreateDefaultSubobject<UGGwaAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UGGwaAttributeSet>("AttributeSet");
	StateComponent = CreateDefaultSubobject<UPlayerStateComponent>("PlayerStateComponent");
}

void AGGwaPlayerState::InitPlayerState() {
	Character = Cast<AGGwaCharacter>(ASC->GetAvatarActor());
	StateComponent->InitComponent(ASC);
}



UAbilitySystemComponent* AGGwaPlayerState::GetAbilitySystemComponent() const {
	return ASC.Get() ? ASC.Get() : nullptr;
}

UPlayerStateComponent* AGGwaPlayerState::GetStateComponent() const {
	return StateComponent.Get();
}

void AGGwaPlayerState::BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue) const{
	// OnAttributeChanged.Broadcast(Attribute, NewValue, SkillData);
	// value 판정을 helper로
	// FObservedAttributeHelper<> Player의 status enum 필요
	if (HasAuthority()) {
		if (Attribute == AttributeSet->GetHealthAttribute()) {
			if (NewValue <= 0.0f) {
				// Component에 요청할것.
				// SetPlayerDeathState();
				ASC->AddLooseGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Dead));
			
			}
		}
	}
}


//server? client? Deprecated -> tag 개념으로 ASC에 위임하여 상태 처리중.
// void AGGwaPlayerState::SetPlayerDeathState() const{
	// StateComponent->OnDeath();
	// if (Character && Character->GetReactionComponent())
	// {
	// 	Character->GetReactionComponent()->ExecuteDeadReaction();
	// }
// }
