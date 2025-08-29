#include "Shared/GAS/GGwaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Utill/UEnumTagMatchHelper.h"

DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, Health)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, MaxHealth)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, Mana)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, MaxMana)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, Speed)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, Defense)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, Critical)
DEFINE_ONREP_ATTRIBUTE(UGGwaAttributeSet, Damage)


UGGwaAttributeSet::UGGwaAttributeSet() {
	InitHealth(100.f);
	InitMana(70.f);
	InitMaxHealth(200.f);
	InitMaxMana(100.f);
	InitDefense(10.f); 
	InitCritical(5.f);  
	InitSpeed(100.f);  
	InitDamage(10.f);
}

void UGGwaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) {
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute()) {
		NewValue = FMath::Clamp(NewValue,0.f,GetMaxHealth());
	}
	if (Attribute == GetManaAttribute()) {
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UGGwaAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) {
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute()) {
		SetHealth(FMath::Clamp(GetHealth(),0.f,GetMaxHealth()));
		if (GetHealth() == 0.f && !GetOwningAbilitySystemComponent()->HasMatchingGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Dead))) {
			//player state component에 위임
			GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Dead)), true);
			GetOwningAbilitySystemComponent()->AddLooseGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Dead));
		}
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute()) {
		SetMana(FMath::Clamp(GetMana(),0.f,GetMaxMana()));
	}
}


void UGGwaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Critical, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Speed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGGwaAttributeSet, Damage, COND_None, REPNOTIFY_Always);
}
