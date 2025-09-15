#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDBProvider.generated.h"

UINTERFACE()
class UDBProvider : public UInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IDBProvider {
	GENERATED_BODY()

public:
	virtual TSharedPtr<class IEquipDBProvider>     GetEquipDbProvider() = 0;
	virtual TSharedPtr<class IShopDBProvider>      GetShopDbProvider() = 0;
	virtual TSharedPtr<class ISkillDBProvider>     GetSkillDbProvider() = 0;
	virtual TSharedPtr<class IInventoryDBProvider> GetInventoryDbProvider() = 0;
};
