// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/ItemDataAsset.h"

#include "CombinedAbilityDataAsset.generated.h"

/**
 * 
 */


UCLASS()
class MYGAME_API UCombinedAbilityDataAsset : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY(VIsibleAnywhere)
	TObjectPtr<USkillDataAsset> SkillDataAsset;
	UPROPERTY(VIsibleAnywhere)
	TObjectPtr<UBuffDataAsset> BuffDataAsset;
	UPROPERTY(VIsibleAnywhere)
	TObjectPtr<UItemDataAsset> ItemDataAsset;

	template<typename T>
	T* GetData() const;
	
	void SetData(USkillDataAsset* SkillData = nullptr, UBuffDataAsset* BuffData = nullptr, UItemDataAsset* ItemData = nullptr);
};

template<typename T>
T* UCombinedAbilityDataAsset::GetData() const
{
	if (TIsSame<T, USkillDataAsset>::Value) return Cast<T>(SkillDataAsset);
	if (TIsSame<T, UBuffDataAsset>::Value) return Cast<T>(BuffDataAsset);
	if (TIsSame<T, UItemDataAsset>::Value) return Cast<T>(ItemDataAsset);
	return nullptr;
}
