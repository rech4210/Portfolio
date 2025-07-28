// @Needmodifi
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEquipmentRepositoryInterface.h"
#include "UObject/Object.h"
#include "EquipmentRepository.generated.h"

class UDatabaseManager;
class UEquipmentComponent;

/**
 * 
 */
UCLASS()
class EQUIPMENTMODULE_API UEquipmentRepository : public UObject, public IEquipmentRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void Initialize() override;
	virtual bool LoadEquipmentData(const FGuid& PlayerGuid, UEquipmentComponent& EquipmentComponentToPopulate) override;
	virtual bool SaveEquipmentData(const FGuid& PlayerGuid, const UEquipmentComponent* EquipmentComponentToSave) override;

private:
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DBManager;
};
