

#include "Shared/AI/EnemyAbilitySystemComponent.h"


UEnemyAbilitySystemComponent::UEnemyAbilitySystemComponent() {
	SetIsReplicated(true);
}

void UEnemyAbilitySystemComponent::BeginPlay() {
	Super::BeginPlay();
	// ?�탯 초기 ?�정???�한 GE, CurveTable.
	//ApplyGameplayEffectToSelf(StartupEffect, 1.f, ASC->MakeEffectContext());
}
