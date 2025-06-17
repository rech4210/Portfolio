// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "EnemyAbilitySystemComponent.generated.h"

/**
 * 
 */		
UCLASS()
class MYGAME_API UEnemyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	UEnemyAbilitySystemComponent();

	virtual void BeginPlay() override;
	
};
