// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Shared/Utill/FRewardData.h"
#include "Shared/Utill/FRewardRequest.h"
#include "ServerLogicBridge.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnFlowComplete, bool /*bOK*/, const FRewardData& /*Data*/, const FString& /*Error*/);

// This class does not need to be modified.
UINTERFACE()
class UServerLogicBridge : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class MYGAME_API IServerLogicBridge {
	GENERATED_BODY()
public:
	virtual void InitiateRewardFlow(const FString& PlayerId, const FRewardRequest& Payload, FOnFlowComplete Callback) = 0;
};
