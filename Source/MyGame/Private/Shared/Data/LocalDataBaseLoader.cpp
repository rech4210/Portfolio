// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Data/LocalDataBaseLoader.h"
#include "Engine/AssetManager.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"

void ULocalDataBaseLoader::Initialize()
{
	UAssetManager& Manager = UAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	Manager.GetPrimaryAssetIdList("Skill", Ids);

	for (const FPrimaryAssetId& Id : Ids)
	{
		// 아직 로드된 적이 없는 경우에만 로드 요청
		if (!Manager.GetPrimaryAssetObject(Id))
		{
			TSharedPtr<FStreamableHandle> Handle =
				Manager.LoadPrimaryAsset(
					Id,
					{},
					FStreamableDelegate(),
					0
				);
			if (Handle.IsValid())
			{
				// 동기 블록킹
				Handle->WaitUntilComplete();
			}
		}

		// 동일한 아이디값일 경우 예외처리
		if (USkillDataAsset* Data = Cast<USkillDataAsset>(Manager.GetPrimaryAssetObject(Id)))
		{
			SkillIdToAssetId.Add(Data->SkillID, Id);
			UE_LOG(LogTemp, Log, TEXT("Mapped SkillID %d → %s"), Data->SkillID, *Id.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load SkillDataAsset for %s"), *Id.ToString());
		}
	}
}


bool ULocalDataBaseLoader::CheckPrimaryAssetId(int32 SkillID, FPrimaryAssetId& OutId) const {
	if (const FPrimaryAssetId* Found = SkillIdToAssetId.Find(SkillID))
	{
		OutId = *Found;
		return true;
	}
	return false;
}
