
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerIdentityInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPlayerIdentityInterface : public UInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IPlayerIdentityInterface {
	GENERATED_BODY()

public:
	virtual FGuid GetPlayerGuid() const = 0;
	virtual void SetPlayerGuid(const FString& UserGuid) = 0;
};
