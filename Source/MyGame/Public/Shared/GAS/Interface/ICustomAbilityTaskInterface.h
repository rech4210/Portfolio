
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "ICustomAbilityTaskInterface.generated.h"

UINTERFACE()
class UCustomAbilityTaskInterface : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class MYGAME_API ICustomAbilityTaskInterface {
	GENERATED_BODY()

public:
	virtual UAbilityTask_WaitTargetData* GetTargetDataTask() = 0;
	virtual void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data) = 0;
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data) = 0;
};
