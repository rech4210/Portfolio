
#pragma once

#include "UObject/Interface.h"
#include "IDBProviderInfra.h"
#include "BaseRepositoryInterface.generated.h"

UINTERFACE()
class GAMESHAREDMODULE_API UBaseRepositoryInterface : public UInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IBaseRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void Initialize(IDBProviderInfra* Infra) = 0;
};
