// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "BaseDataAsset.generated.h"

class UUserWidget;
/**
 * 
 */
UCLASS()
class MYGAME_API UBaseDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UUserWidget* Tooltip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag UniqueTag;  // 고유 식별용
};
