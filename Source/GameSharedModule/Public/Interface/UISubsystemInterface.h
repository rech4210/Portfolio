
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UISubsystemInterface.generated.h"
class AUIConfigCacheActor;
UINTERFACE()
class UUISubsystemInterface : public UInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IUISubsystemInterface {
	GENERATED_BODY()

public:
	virtual void SetCacheActor(AUIConfigCacheActor * Actor) = 0;
};
