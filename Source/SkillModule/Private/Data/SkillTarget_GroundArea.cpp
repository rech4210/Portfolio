
#include "Data/SkillTarget_GroundArea.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Data/SkillDataAsset.h"


TArray<AActor*> USkillTarget_GroundArea::DetectTargets(FSkillContext& SkillContext) {
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

	const FVector& TargetLocation = SkillContext.TargetLocation;
	const float Radius = SkillContext.SkillData->SkillShapeConfig.SphereRadius;
	const ECollisionChannel TraceChannel = SkillContext.SkillData->SkillShapeConfig.TraceChannel;

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(SkillContext.SourceActor);

	bool bOverlap = World->OverlapMultiByChannel(
		OverlapResults,
		TargetLocation,
		FQuat::Identity,
		TraceChannel,
		Shape,
		QueryParams
	);

	if (bOverlap)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && HasASC(HitActor))
			{
				DetectedActors.Add(HitActor);
			}
		}
	}
	
	return DetectedActors;
}
