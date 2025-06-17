// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/AI/EnemyAbilitySystemComponent.h"


UEnemyAbilitySystemComponent::UEnemyAbilitySystemComponent() {
	SetIsReplicated(true);
}

void UEnemyAbilitySystemComponent::BeginPlay() {
	Super::BeginPlay();
	// 스탯 초기 설정을 위한 GE, CurveTable.
	//ApplyGameplayEffectToSelf(StartupEffect, 1.f, ASC->MakeEffectContext());
}
