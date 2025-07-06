// Fill out your copyright notice in the Description page of Project Settings.


#include "Repositories/SkillStateRepository.h"
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"
#include "Components/SkillComponent.h"

bool USkillStateRepository::LoadSkillState(int32 PlayerInformation, USkillComponent& SkillComponentToPopulate, TArray<int32> fetchedSkillList) {
	// 서버 권한이 있을 때만 복제된 프로퍼티를 수정
	if (!SkillComponentToPopulate.GetOwner() || !SkillComponentToPopulate.GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillStateRepository: LoadSkillState can only be called on server authority"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SkillStateRepository: Loading skill state for Player %d with %d skills"), 
		PlayerInformation, fetchedSkillList.Num());

	for (int32 SkillIndex : fetchedSkillList) {
		FPrimaryAssetId AssetId;
		ULocalDataBaseLoader::CheckPrimaryAssetId(SkillIndex, AssetId);
		auto SkillData = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetId);
		if (!SkillData) {
			UE_LOG(LogTemp, Warning, TEXT("SkillStateRepository: Failed to load skill data for SkillIndex %d"), SkillIndex);
			continue; // 실패한 스킬은 건너뛰고 계속 진행
		}
		
		// 스킬 등록 - 이것이 복제를 트리거함 (서버 권한 하에서만)
		if (!SkillComponentToPopulate.RegisterSkill(SkillData)) {
			UE_LOG(LogTemp, Warning, TEXT("SkillStateRepository: Failed to register skill %d"), SkillIndex);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("SkillStateRepository: Successfully registered skill %d"), SkillIndex);
		}
	}
	
	ULocalDataBaseLoader::CheckRegistedAsset();
	UE_LOG(LogTemp, Log, TEXT("SkillStateRepository: Completed loading skills for Player %d"), PlayerInformation);
	return true;
}

bool USkillStateRepository::SaveSkillState(int32 PlayerInformation, const USkillComponent* SkillComponentToSave, TArray<int32> SkillPayloadList) {
	return true;
}
