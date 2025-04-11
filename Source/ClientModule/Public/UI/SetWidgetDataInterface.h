// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SetWidgetDataInterface.generated.h"

// This class does not need to be modified.
class UPrimaryDataAsset;
class UBuffDataAsset;
class USkillDataAsset;
class UItemDataAsset;
UINTERFACE()
class USetWidgetDataInterface : public UInterface {
	GENERATED_BODY()
};

class CLIENTMODULE_API ISetWidgetDataInterface {
	GENERATED_BODY()

public:
	virtual void SetWidgetData(UPrimaryDataAsset* Data) = 0;
};
