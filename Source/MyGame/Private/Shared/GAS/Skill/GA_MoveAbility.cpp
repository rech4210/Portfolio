// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Skill/GA_MoveAbility.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaCharacter.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"


void UGA_MoveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	TWeakObjectPtr<AGGwaCharacter> Avatar = Cast<AGGwaCharacter>(ActorInfo->AvatarActor.Get());
	TWeakObjectPtr<AGGwaPlayerController> PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController);

	if (!Avatar.Get() || !PC.Get()) return;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		FRotator HitRotation = FRotator::ZeroRotator;
		GetWorld()->SpawnActor<AActor>(LineTraceRing, Hit.ImpactPoint, HitRotation);

		UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
			GetWorld(), Avatar->GetActorLocation(), Hit.ImpactPoint);

		if (Path && Path->PathPoints.Num() > 1)
		{
			Avatar->SetMoveData(Path->PathPoints,1, true);
			// Debug Lines
			for (int32 i = 0; i < Path->PathPoints.Num() - 1; ++i)
			{
				DrawDebugLine(GetWorld(), Path->PathPoints[i], Path->PathPoints[i + 1], FColor::Green, false, 5.0f, 0, 5.0f);
				DrawDebugSphere(GetWorld(), Path->PathPoints[i], 20.0f, 12, FColor::Red, false, 5.0f);
			}
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
