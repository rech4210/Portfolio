// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/SkillTarget_Projectile.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Data/SkillDataAsset.h"

TArray<AActor*> USkillTarget_Projectile::DetectTargets(FSkillContext& SkillContext)
{
	TArray<AActor*> DetectedActors;

	if (!SkillContext.SourceActor || !SkillContext.SkillData)
	{
		return DetectedActors;
	}

	UWorld* World = SkillContext.SourceActor->GetWorld();
	if (!World)
	{
		return DetectedActors;
	}

	const FVector StartLocation = SkillContext.SourceActor->GetActorLocation();
	const FVector EndLocation = StartLocation + SkillContext.SourceActor->GetActorForwardVector() * SkillContext.SkillData->SkillShapeConfig.TraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SkillContext.SourceActor);

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		SkillContext.SkillData->SkillShapeConfig.TraceChannel,
		QueryParams
	);

	if (bHit && HitResult.GetActor() && HasASC(HitResult.GetActor()))
	{
		DetectedActors.Add(HitResult.GetActor());
	}

	// For debugging
	// DrawDebugLine(World, StartLocation, EndLocation, FColor::Red, false, 2.0f);

	return DetectedActors;
}
