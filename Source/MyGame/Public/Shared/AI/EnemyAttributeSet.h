// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "EnemyAttributeSet.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#define DEFINE_ONREP_ATTRIBUTE(ClassName, PropertyName) \
void ClassName::OnRep_##PropertyName(const FGameplayAttributeData& OldValue) const \
{ \
GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, PropertyName, OldValue); \
}

UCLASS()
class MYGAME_API UEnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UEnemyAttributeSet();

	UPROPERTY(EditAnywhere, Category = "Attribute")
	float DefaultHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Attribute")
	float DefaultMaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Attribute")
	float DefaultDamage = 10.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, MaxHealth)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, Damage)


	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_Damage(const FGameplayAttributeData& Old) const;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
