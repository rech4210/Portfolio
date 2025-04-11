// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GGwaAbilitySystemComponent.h"


UGGwaAbilitySystemComponent::UGGwaAbilitySystemComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}
