
#pragma once

#include "CoreMinimal.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Subsystems/WorldSubsystem.h"
#include "UServerCoreSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SERVERMODULE_API UServerCoreSubsystem : public UWorldSubsystem, public IServerLogicBridge {
	GENERATED_BODY()

public:
	virtual void InitiateRewardFlow(const FString& PlayerId, const FRewardRequest& Payload,FOnFlowComplete Callback) override;
};
