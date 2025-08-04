
#pragma once

#include "CoreMinimal.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "UObject/Interface.h"
#include "EnemyDataReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UEnemyDataReceiver : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class MYGAME_API IEnemyDataReceiver {
	GENERATED_BODY()

public:
	virtual void ReceiveEnemyData(FBossDataStruct& Data) = 0;
};
