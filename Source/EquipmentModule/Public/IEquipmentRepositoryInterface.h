
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Async/Future.h"
#include "Interface/BaseRepositoryInterface.h"
#include "IEquipmentRepositoryInterface.generated.h"

class UEquipmentComponent;

UINTERFACE()
class UEquipmentRepositoryInterface : public UBaseRepositoryInterface {
	GENERATED_BODY()
};

class EQUIPMENTMODULE_API IEquipmentRepositoryInterface : public IBaseRepositoryInterface {
	GENERATED_BODY()

public:
	virtual bool LoadEquipmentData(const FGuid& PlayerGuid, UEquipmentComponent& EquipmentComponentToPopulate) = 0;
	virtual bool SaveEquipmentData(const FGuid& PlayerGuid, const UEquipmentComponent* EquipmentComponentToSave) = 0;
};
