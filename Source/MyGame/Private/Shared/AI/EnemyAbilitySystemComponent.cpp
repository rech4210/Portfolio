// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/AI/EnemyAbilitySystemComponent.h"


UEnemyAbilitySystemComponent::UEnemyAbilitySystemComponent() {
	SetIsReplicated(true);
}

void UEnemyAbilitySystemComponent::BeginPlay() {
	Super::BeginPlay();
	// ?¤íƒ¯ ì´ˆê¸° ?¤ì •???„í•œ GE, CurveTable.
	//ApplyGameplayEffectToSelf(StartupEffect, 1.f, ASC->MakeEffectContext());
}
