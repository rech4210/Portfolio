// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "IEquipmentRepositoryInterface.generated.h"

class UEquipmentComponent;

// This class does not need to be modified.
UINTERFACE()
class UEquipmentRepositoryInterface : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class EQUIPMENTMODULE_API IEquipmentRepositoryInterface {
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Loads equipment data for a specific player/equipment component.
	 * @param PlayerInformation Player ID or identifier
	 * @param EquipmentComponentToPopulate The equipment component to populate with data
	 * @return True if loading was successful
	 */
	virtual bool LoadEquipmentData(int32 PlayerInformation, UEquipmentComponent& EquipmentComponentToPopulate) = 0;

	/**
	 * Saves equipment data from an equipment component.
	 * @param PlayerInformation Player ID or identifier
	 * @param EquipmentComponentToSave The equipment component containing data to save
	 * @return True if saving was successful
	 */
	virtual bool SaveEquipmentData(int32 PlayerInformation, const UEquipmentComponent* EquipmentComponentToSave) = 0;
};
