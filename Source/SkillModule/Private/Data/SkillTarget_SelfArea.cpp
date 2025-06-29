
#include "Data/SkillTarget_SelfArea.h"
#include "Data/SkillDataAsset.h"

TArray<AActor*> USkillTarget_SelfArea::DetectTargets(FSkillContext& SkillContext) {
	UE_LOG(LogTemp, Warning, TEXT("SkillTarget: TargetStrategySelfArea"));
	TArray<AActor*> Actors;

	const FSkillShapeConfig& Config = SkillContext.SkillData->SkillShapeConfig;
	UWorld* World = SkillContext.SourceActor->GetWorld();
	if (!World)
	{
		return Actors;
	}
    
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SkillContext.SourceActor);

	TArray<FOverlapResult> Overlaps;

	// 자신을 기준으로 원형 범위 설정
	bool bOverlap = World->OverlapMultiByChannel(
		Overlaps,
		SkillContext.SourceActor->GetActorLocation(), // 자신의 위치를 기준으로
		FQuat::Identity,
		Config.TraceChannel,
		FCollisionShape::MakeSphere(Config.SphereRadius), // FSkillShapeConfig의 SphereRadius 사용
		Params
	);

	if (bOverlap) {
		for (const FOverlapResult& OverlapResult : Overlaps) {
			AActor* OverlappedActor = OverlapResult.GetActor();
			if (OverlappedActor && !Actors.Contains(OverlappedActor))
				Actors.Add(OverlapResult.GetActor());
		}
	}

	DebugSkillShape(World, FVector::Zero(), SkillContext.SourceActor->GetActorLocation(), Config);
    
	return Actors;
}
