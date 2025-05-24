// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Shared/Data/BaseDataAsset.h"
#include "LocalDataBaseLoader.generated.h"

DECLARE_DELEGATE_OneParam(FOnSkillDataLoaded, UBaseDataAsset*);

/**
 * 
 */
class UCombinedAbilityDataAsset;
UCLASS()
class MYGAME_API ULocalDataBaseLoader : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	bool GetPrimaryAssetId(int32 SkillID, FPrimaryAssetId& OutId) const;

	// AssetId 로부터 실제 DataAsset 꺼내기 (동기)
	template<typename T>
	T* GetDataFromAssetId(const FPrimaryAssetId& AssetId, bool bSync = true){
		UAssetManager& Manager = UAssetManager::Get();

		if (UObject* Obj = Manager.GetPrimaryAssetObject(AssetId)){
			return Cast<T>(Obj);
		}

		// 비동기 로드시 콜백 필요
		Manager.LoadPrimaryAsset(
			AssetId,{},FStreamableDelegate(),0
		);

		return Cast<T>( Manager.GetPrimaryAssetObject(AssetId));
	}
private:
	TMap<int32, FPrimaryAssetId> SkillIdToAssetId;
};