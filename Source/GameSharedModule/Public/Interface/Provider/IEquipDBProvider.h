// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IEquipDBProvider.generated.h"

UINTERFACE()
class UEquipDBProvider : public UInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IEquipDBProvider {
	GENERATED_BODY()
public:
	// virtual bool LoadEquipmentData(const FGuid& PlayerGuid, class UEquipmentComponent& EquipmentComponentToPopulate) = 0;
	// virtual bool SaveEquipmentData(const FGuid& PlayerGuid, const UEquipmentComponent* EquipmentComponentToSave) = 0;
};
