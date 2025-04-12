// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Self.h"

TArray<AActor*> USkillTarget_Self::DetectTargets(FSkillContext& SkillContext) {
	TArray<AActor*> Actors;

	if (SkillContext.SourceActor) {
		Actors.Add(SkillContext.SourceActor);
	}
	return Actors;
}
