
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IEquipmentRepositoryInterface.h"
#include "Interface/PlayerIdentityInterface.h"
#include "EquipmentSubsystem.generated.h"

class UEquipmentRepository;
class UEquipmentComponent;

UCLASS()
class EQUIPMENTMODULE_API UEquipmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<IEquipmentRepositoryInterface> GetEquipmentRepository() const;
	void RequestLoadEquipmentData(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);
	void Client_OnEquipmentStateUpdated(UEquipmentComponent* EquipmentComponent);

private:
	UPROPERTY()
	UEquipmentRepository* EquipmentRepository;
};
