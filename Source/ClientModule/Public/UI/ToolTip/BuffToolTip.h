// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseToolTip.h"
#include "BuffToolTip.generated.h"

class UPrimaryDataAsset;
/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UBuffToolTip : public UBaseToolTip {
	GENERATED_BODY()

public:
	virtual void SetToolTipData(UPrimaryDataAsset* Data) override;
};
