// Fill out your copyright notice in the Description page of Project Settings.


#include "Repositories/SkillConfigRepository.h"

void USkillConfigRepository::LoadSkillDefinitions(TArray<USkillDataAsset*>& OutSkillDefinitions) {
	//
	TArray<int32> SkillIDs = { 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110 };
	for (int32 ID : SkillIDs) {
		// local data base loader or add here all skill data...
		// OutSkillDefinitions.Add();
	}
}
