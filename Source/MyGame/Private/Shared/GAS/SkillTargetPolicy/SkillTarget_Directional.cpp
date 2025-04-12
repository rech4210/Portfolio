// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Directional.h"


TArray<AActor*> USkillTarget_Directional::DetectTargets(FSkillContext& SkillContext) {
	TArray<AActor*> Actors;

	if (!SkillContext.SourceActor || !SkillContext.SkillData)
	{
		return Actors;
	}

	const UWorld* World = SkillContext.SourceActor->GetWorld();
	if (!World)
	{
		return Actors;
	}

	const FSkillShapeConfig& Config = SkillContext.SkillData->SkillShapeConfig;

	const FVector StartLocation = SkillContext.SourceActor->GetActorLocation();
	const FVector ForwardVector = SkillContext.SourceActor->GetActorForwardVector();
	const FVector EndLocation = StartLocation + (ForwardVector * Config.TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SkillContext.SourceActor);

	// 📦 BoxTrace 감지
	TArray<FHitResult> HitResults;

	bool bHit = World->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		Config.TraceChannel,
		FCollisionShape::MakeBox(Config.BoxHalfExtent),
		Params
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !Actors.Contains(HitActor))
			{
				Actors.Add(HitActor);
			}
		}
	}

	DebugSkillShape(World, StartLocation, EndLocation, Config);

	// 🔵 SphereOverlap 감지
	TArray<FOverlapResult> Overlaps;

	bool bOverlap = World->OverlapMultiByChannel(
		Overlaps,
		EndLocation,
		FQuat::Identity,
		Config.TraceChannel,
		FCollisionShape::MakeSphere(Config.SphereRadius),
		Params
	);

	if (bOverlap)
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* OverlappedActor = Result.GetActor();
			if (OverlappedActor && !Actors.Contains(OverlappedActor))
			{
				Actors.Add(OverlappedActor);
			}
		}
	}
	
	return Actors;
}
