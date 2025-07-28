// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#include "Utill/LocalDataBaseLoader.h"
#include "Engine/AssetManager.h"
#include "Data/SkillDataAsset.h"

TMap<int32, FPrimaryAssetId> ULocalDataBaseLoader::SkillIdToAssetId;
bool ULocalDataBaseLoader::bIsInitialized = false;
FOnSkillDataLoadedSignature ULocalDataBaseLoader::OnSkillDataLoaded;

void ULocalDataBaseLoader::Initialize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("LocalDataBaseLoader is already initialized"));
        return;
    }

    if (!GEngine || !GEngine->AssetManager)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalDataBaseLoader::Initialize failed: AssetManager not ready"));
        return;
    }

    UAssetManager& Manager = UAssetManager::Get();
    TArray<FPrimaryAssetId> Ids;
	Manager.GetPrimaryAssetIdList("Skill", Ids);

	for (const FPrimaryAssetId& Id : Ids)
	{
		// ?�직 로드???�이 ?�는 경우?�만 로드 ?�청
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
				// ?�기 블록??
				Handle->WaitUntilComplete();
			}
		}

		if (USkillDataAsset* Data = Cast<USkillDataAsset>(Manager.GetPrimaryAssetObject(Id)))
		{
			SkillIdToAssetId.Add(Data->SkillID, Id);
			UE_LOG(LogTemp, Log, TEXT("Mapped SkillID %d ??%s"), Data->SkillID, *Id.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load SkillDataAsset for %s"), *Id.ToString());
		}
	}
	
	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("LocalDataBaseLoader successfully initialized with %d skills"), SkillIdToAssetId.Num());
}

void ULocalDataBaseLoader::CheckRegistedAsset() {
	for (auto Element : SkillIdToAssetId) {
		UE_LOG(LogTemp, Log, TEXT("Regist Skill %s"), *GetDataFromAssetId<USkillDataAsset>(Element.Value)->DisplayName.ToString());
	}
}

bool ULocalDataBaseLoader::CheckPrimaryAssetId(int32 SkillID, FPrimaryAssetId& OutId){
	if (const FPrimaryAssetId* Found = SkillIdToAssetId.Find(SkillID)){
		OutId = *Found;
		return true;
	}
	return false;
}
