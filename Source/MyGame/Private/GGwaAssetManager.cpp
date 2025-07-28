// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.


#include "GGwaAssetManager.h"
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"

const FPrimaryAssetType UGGwaAssetManager::SkillType = TEXT("Skill");

void UGGwaAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	// Initialize LocalDataBaseLoader before any asset loading
}

void UGGwaAssetManager::FinishInitialLoading() {
	Super::FinishInitialLoading();
	
	// Initialize check
	// ULocalDataBaseLoader::Initialize();
	// if (!ULocalDataBaseLoader::IsInitialized())
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("LocalDataBaseLoader is not initialized! Skipping prefetch."));
	// 	return;
	// }

	// 프리페치할 스킬 ID들
	// 해당 부분을 관리하기 쉽도록 Enum 또는 Map으로 설정.
	// TArray<int32> PrefetchSkillIDs = { 100,101,102,103,104,105,106,107 };
	// for (int32 SkillID : PrefetchSkillIDs)
	// {
	// 	FPrimaryAssetId IdPtr;
	// 	if (ULocalDataBaseLoader::CheckPrimaryAssetId(SkillID, IdPtr))
	// 	{
	// 		LoadPrimaryAsset(IdPtr, {}, FStreamableDelegate());
	// 	}
	// 	else 
	// 	{
	//  			UE_LOG(LogTemp, Warning, TEXT("Loading skill %d failed"), SkillID);
	//  	}
	// }
}
