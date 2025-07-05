// Fill out your copyright notice in the Description page of Project Settings.


#include "Repositories/SkillStateRepository.h"
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"
#include "Components/SkillComponent.h"

bool USkillStateRepository::LoadSkillState(int32 PlayerInformation, USkillComponent& SkillComponentToPopulate, TArray<int32> fetchedSkillList) {
	for (int32 SkillIndex : fetchedSkillList) {
		FPrimaryAssetId AssetId;
		ULocalDataBaseLoader::CheckPrimaryAssetId(SkillIndex, AssetId);
		auto SkillData = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetId);
		if (!SkillData) {
			UE_LOG(LogTemp, Warning, TEXT("SkillStateRepository: Failed to load skill data for SkillIndex %d"), SkillIndex);
			ULocalDataBaseLoader::CheckRegistedAsset();
			return false;
		}
		
		// 스킬 등록 - 이것이 복제를 트리거함
		if (!SkillComponentToPopulate.RegisterSkill(SkillData)) {
			UE_LOG(LogTemp, Warning, TEXT("SkillStateRepository: Failed to register skill %d"), SkillIndex);
			return false;
		}
	}
	
	return true;
}

bool USkillStateRepository::SaveSkillState(int32 PlayerInformation, const USkillComponent* SkillComponentToSave, TArray<int32> SkillPayloadList) {
	return true;
}
