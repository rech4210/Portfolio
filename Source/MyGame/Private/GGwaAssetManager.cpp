// Fill out your copyright notice in the Description page of Project Settings.


#include "GGwaAssetManager.h"

#include "Shared/Data/LocalDataBaseLoader.h"

const FPrimaryAssetType UGGwaAssetManager::SkillType = TEXT("Skill");

void UGGwaAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

}

void UGGwaAssetManager::FinishInitialLoading() {
	Super::FinishInitialLoading();
	// TArray<FPrimaryAssetId> SkillAssets;
	//
	// // 1) 매핑 먼저 완료
	// if (ULocalDataBaseLoader* Loader = NewObject<ULocalDataBaseLoader>(this))
	// {
	// 	Loader->Initialize();
	// 	TArray<int32> PrefetchSkillIDs = { 15,101,102 };
	// 	for (int32 SkillID : PrefetchSkillIDs)
	// 	{
	// 		FPrimaryAssetId IdPtr;
	// 		Loader->GetPrimaryAssetId(SkillID, IdPtr);
	// 		if (IdPtr.IsValid()){
	// 			LoadPrimaryAsset(IdPtr, {}, FStreamableDelegate());
	// 			SkillAssets.Add(IdPtr);
	// 		}
	// 		else {
	// 			UE_LOG(LogTemp, Warning, TEXT("Loading skill %d failed"), SkillID);
	// 		}
	// 	}
	// }
	//
	// GetPrimaryAssetIdList(FPrimaryAssetType("Skill"), SkillAssets);
	// UE_LOG(LogTemp, Warning, TEXT("[디버깅] 스캔된 Skill 에셋 수: %d"), SkillAssets.Num());
}
