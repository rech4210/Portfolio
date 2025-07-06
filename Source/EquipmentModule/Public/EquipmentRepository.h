// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEquipmentRepositoryInterface.h"
#include "UObject/Object.h"
#include "EquipmentRepository.generated.h"

class UEquipmentComponent;

/**
 * 
 */
UCLASS()
class EQUIPMENTMODULE_API UEquipmentRepository : public UObject, public IEquipmentRepositoryInterface {
	GENERATED_BODY()

public:
	virtual bool LoadEquipmentData(int32 PlayerInformation, UEquipmentComponent& EquipmentComponentToPopulate) override;
	virtual bool SaveEquipmentData(int32 PlayerInformation, const UEquipmentComponent* EquipmentComponentToSave) override;
};
