// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/SkillTargetPolicy/SkillTarget_TargetActor.h"

TArray<AActor*> USkillTarget_TargetActor::DetectTargets(FSkillContext& SkillContext) {
	TArray<AActor*> Actors;
	if (SkillContext.TargetActor){
		Actors.Add(SkillContext.TargetActor.Get());
	}
	return Actors;
}
