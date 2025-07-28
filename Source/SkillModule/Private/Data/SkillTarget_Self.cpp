
#include "Data/SkillTarget_Self.h"
#include "Data/SkillDataAsset.h"

TArray<AActor*> USkillTarget_Self::DetectTargets(FSkillContext& SkillContext) {
	TArray<AActor*> Actors;

	if (SkillContext.SourceActor) {
		Actors.Add(SkillContext.SourceActor);
	}

	UE_LOG(LogTemp, Log, TEXT("SkillTarget: TargetStrategySelf"));
	return Actors;
}
