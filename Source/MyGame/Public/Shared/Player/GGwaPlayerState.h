// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "EPlayerState.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerState.h"
#include "Shared/Data/SkillDataAsset.h"
#include "GGwaPlayerState.generated.h"

class AGGwaCharacter;
class UPlayerStateComponent;
class UGGwaAttributeSet;
class UGGwaAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayAttribute, Attribute, float, NewValue, USkillDataAsset*, SkillData);

/**
 * 
 */
UCLASS()
class MYGAME_API AGGwaPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGGwaPlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable)
	UPlayerStateComponent* GetStateComponent() const; 

	UPROPERTY(BlueprintAssignable, Category = "GAS")
	FOnAttributeChanged OnAttributeChanged;

	void BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue) const;
	void InitPlayerState();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
	
	UPROPERTY()
	TObjectPtr<UGGwaAttributeSet> AttributeSet;
	
	UPROPERTY(VisibleAnywhere, Category="State")
	TObjectPtr<UPlayerStateComponent> StateComponent;
private:
	UPROPERTY()
	TObjectPtr<AGGwaCharacter> Character;
};
