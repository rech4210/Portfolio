// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/SkillTargetPolicy/SkillTarget_MultiBoxTrace.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Shared/Data/SkillDataAsset.h"


TArray<AActor*> USkillTarget_MultiBoxTrace::DetectTargets(FSkillContext& SkillContext) {
	TArray<AActor*> DetectedActors;
	TSet<AActor*> UniqueActors;

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
	const FVector HalfSize = SkillContext.SkillData->SkillShapeConfig.BoxHalfExtent;
	const FRotator Orientation = SkillContext.SourceActor->GetActorRotation();
	const ECollisionChannel TraceChannel = SkillContext.SkillData->SkillShapeConfig.TraceChannel;
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SkillContext.SourceActor);
	FSkillShapeConfig Config = SkillContext.SkillData->SkillShapeConfig;
	bool bHit = World->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		Config.TraceChannel,
		FCollisionShape::MakeBox(Config.BoxHalfExtent),
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HasASC(HitActor))
			{
				UniqueActors.Add(HitActor);
			}
		}
	}
	
	DetectedActors.Append(UniqueActors.Array());
	return DetectedActors;
}
