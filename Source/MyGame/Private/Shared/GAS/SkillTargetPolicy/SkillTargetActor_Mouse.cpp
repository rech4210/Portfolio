// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/SkillTargetPolicy/SkillTargetActor_Mouse.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Shared/Player/GGwaPlayerController.h"

void ASkillTargetActor_Mouse::StartTargeting(UGameplayAbility* InAbility)
{
	Super::StartTargeting(InAbility);
	PlayerController = Cast<AGGwaPlayerController>(InAbility->GetCurrentActorInfo()->PlayerController);
}


void ASkillTargetActor_Mouse::ConfirmTargetingAndContinue()
{
	if (!PlayerController) return;

	FHitResult Hit;
	PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (!Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mouse targeting failed - no blocking hit"));
		return;
	}

	FGameplayAbilityTargetDataHandle Data = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
	if (!Data.IsValid(0)) return;

	TargetDataReadyDelegate.Broadcast(Data);
}
