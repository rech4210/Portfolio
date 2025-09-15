#pragma once
#include "Interface/Provider/IEquipDBProvider.h"
#include "DatabaseManager.h"

/*현재는 구현체가 직접 API를 구성하려 하지만, GameShared Interface를 통해서 API를 정의하고 GameMode에는 인터페이스를 노출하는걸로*/
class EquipDBProvider : public IEquipDBProvider {
public:
	EquipDBProvider(UDatabaseManager* InDBManager): DBManager(InDBManager) {}
	// virtual bool LoadEquipmentData(const FGuid& PlayerGuid, class UEquipmentComponent& EquipmentComponentToPopulate) override;
	// virtual bool SaveEquipmentData(const FGuid& PlayerGuid, const class UEquipmentComponent* EquipmentComponentToSave) override;
	/*DB APIs from interface*/
private:
	TWeakObjectPtr<UDatabaseManager> DBManager;
};
