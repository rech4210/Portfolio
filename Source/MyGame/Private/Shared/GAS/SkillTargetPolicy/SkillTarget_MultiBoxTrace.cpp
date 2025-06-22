// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/SkillTargetPolicy/SkillTarget_MultiBoxTrace.h"

TArray<AActor*> USkillTarget_MultiBoxTrace::DetectTargets(FSkillContext& SkillContext) {
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
        const FVector Forward = SkillContext.SourceActor->GetActorForwardVector();
        const FVector EndLocation = StartLocation + Forward * Config.TraceDistance;

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(SkillContext.SourceActor);

        int32 NumSteps = 3;
        FVector Step = (EndLocation - StartLocation) / NumSteps;
        for (int32 i = 1; i <= NumSteps; ++i)
        {
                FVector Center = StartLocation + Step * i;
                TArray<FHitResult> Hits;
                bool bHit = World->SweepMultiByChannel(
                        Hits,
                        Center,
                        Center,
                        FQuat::Identity,
                        Config.TraceChannel,
                        FCollisionShape::MakeBox(Config.BoxHalfExtent),
                        Params);

                if (bHit)
                {
                        for (const FHitResult& Hit : Hits)
                        {
                                AActor* HitActor = Hit.GetActor();
                                if (HitActor && !Actors.Contains(HitActor) && HasASC(HitActor))
                                {
                                        Actors.Add(HitActor);
                                }
                        }
                }
        }

        DebugSkillShape(World, StartLocation, EndLocation, Config);
        return Actors;
}
