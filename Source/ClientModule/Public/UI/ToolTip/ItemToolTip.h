// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseToolTip.h"
#include "ItemToolTip.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UItemToolTip : public UBaseToolTip {
	GENERATED_BODY()
public:
	virtual void SetToolTipData(UPrimaryDataAsset* Data) override;
};
