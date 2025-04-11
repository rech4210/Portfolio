// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseDataAsset.h"
#include "ItemDataAsset.generated.h"
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Consumable,
	Equipment,
	Quest,
	Misc
};

/**
 * 
 */
class UGameplayEffect;
UCLASS()
class MYGAME_API UItemDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStackCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> UseEffect;

};
