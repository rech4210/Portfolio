

#include "Shared/AI/EnemyAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"

UEnemyAttributeSet::UEnemyAttributeSet() {
	InitHealth(DefaultHealth);
	InitMaxHealth(DefaultMaxHealth);
	InitDamage(DefaultDamage);
}

DEFINE_ONREP_ATTRIBUTE(UEnemyAttributeSet, Health)
DEFINE_ONREP_ATTRIBUTE(UEnemyAttributeSet, MaxHealth)
DEFINE_ONREP_ATTRIBUTE(UEnemyAttributeSet, Damage)

void UEnemyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) {
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute()) {
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UEnemyAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) {
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetHealthAttribute()) {
		SetHealth(FMath::Clamp(GetHealth(),0.f,GetMaxHealth()));
	}
}


void UEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, Damage, COND_None, REPNOTIFY_Always);
}
