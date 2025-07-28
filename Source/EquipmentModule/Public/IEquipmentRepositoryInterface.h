
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "Interface/BaseRepositoryInterface.h"
#include "IEquipmentRepositoryInterface.generated.h"

class UEquipmentComponent;

// This class does not need to be modified.
UINTERFACE()
class UEquipmentRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class EQUIPMENTMODULE_API IEquipmentRepositoryInterface : public IBaseRepositoryInterface {
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Loads equipment data for a specific player/equipment component.
	 * @param PlayerGuid Player GUID or identifier
	 * @param EquipmentComponentToPopulate The equipment component to populate with data
	 * @return True if loading was successful
	 */
	virtual bool LoadEquipmentData(const FGuid& PlayerGuid, UEquipmentComponent& EquipmentComponentToPopulate) = 0;

	/**
	 * Saves equipment data from an equipment component.
	 * @param PlayerGuid Player GUID or identifier
	 * @param EquipmentComponentToSave The equipment component containing data to save
	 * @return True if saving was successful
	 */
	virtual bool SaveEquipmentData(const FGuid& PlayerGuid, const UEquipmentComponent* EquipmentComponentToSave) = 0;
};
