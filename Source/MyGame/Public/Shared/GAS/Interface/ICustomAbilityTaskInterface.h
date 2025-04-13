// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "ICustomAbilityTaskInterface.generated.h"

// This class does not need to be modified.
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
