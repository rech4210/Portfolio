// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GGwaPlayerState.generated.h"

class UGGwaAttributeSet;
class UAbilitySystemComponent;
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
protected:
	// 아직 UMyAbilitySystemComponent는 안씀.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	UPROPERTY()
	TObjectPtr<UGGwaAttributeSet> AttributeSet;
};
