
#pragma once

#include "CoreMinimal.h"
#include "../../GameSharedModule/Public/Interface/IEquipmentRepositoryInterface.h"
#include "UObject/Object.h"
#include "EquipmentRepository.generated.h"

class UEquipmentComponent;
class IEquipDBProvider;

UCLASS()
class EQUIPMENTMODULE_API UEquipmentRepository : public UObject, public IEquipmentRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void Initialize(IDBProviderInfra* Infra) override;
	virtual bool LoadEquipmentData(const FGuid& PlayerGuid, UEquipmentComponent& EquipmentComponentToPopulate) override;
	virtual bool SaveEquipmentData(const FGuid& PlayerGuid, const UEquipmentComponent* EquipmentComponentToSave) override;

private:
	TSharedPtr<IEquipDBProvider> EquipmentDBProvider;
};
