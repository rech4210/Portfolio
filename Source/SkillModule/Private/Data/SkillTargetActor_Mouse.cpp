// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillModule/Public/Data/SkillTargetActor_Mouse.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"

void ASkillTargetActor_Mouse::StartTargeting(UGameplayAbility* InAbility)
{
	Super::StartTargeting(InAbility);
	PlayerController = InAbility->GetCurrentActorInfo()->PlayerController.Get();
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

	DrawDebugCone(GetWorld(),
		Hit.Location,
		PlayerController->GetPawn()->GetActorForwardVector(),
		50.f,
		100.f,
		50.f,
		20.f,
		FColor::Red,
		false,
		5.f,
		0,
		1.f
	);
	UE_LOG(LogTemp, Warning, TEXT("Mouse ImpactPoint x- %f"), Hit.ImpactPoint.X);
	UE_LOG(LogTemp, Warning, TEXT("Mouse ImpactPoint  y- %f"), Hit.ImpactPoint.Y);
	UE_LOG(LogTemp, Warning, TEXT("Mouse ImpactPoint   z- %f"), Hit.ImpactPoint.Z);

	UE_LOG(LogTemp, Warning, TEXT("Mouse Location  X- %f"), Hit.Location.X);
	UE_LOG(LogTemp, Warning, TEXT("Mouse Location   Y- %f"), Hit.Location.Y);
	UE_LOG(LogTemp, Warning, TEXT("Mouse Location    Z- %f"), Hit.Location.Z);


	FGameplayAbilityTargetDataHandle Data = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
	if (!Data.IsValid(0)) return;

	TargetDataReadyDelegate.Broadcast(Data);
}
