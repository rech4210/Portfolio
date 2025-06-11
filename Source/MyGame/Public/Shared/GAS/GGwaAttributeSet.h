// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GGwaAttributeSet.generated.h"

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
class MYGAME_API UGGwaAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGGwaAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, MaxMana)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, Defense)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Critical)
	FGameplayAttributeData Critical;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, Critical)

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Speed)
	FGameplayAttributeData Speed;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, Speed)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UGGwaAttributeSet, Damage)


	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_Critical(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_Speed(const FGameplayAttributeData& Old) const;
	UFUNCTION()
	void OnRep_Damage(const FGameplayAttributeData& Old) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};

