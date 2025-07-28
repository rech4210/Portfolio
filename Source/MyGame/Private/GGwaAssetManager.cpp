

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

	// ������ġ�� ��ų ID��
	// �ش� �κ��� �����ϱ� ������ Enum �Ǵ� Map���� ����.
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
