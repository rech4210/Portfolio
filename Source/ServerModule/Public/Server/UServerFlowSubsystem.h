
#pragma once

#include "CoreMinimal.h"
#include "UPlayerValidationSubsystem.h"
#include "URewardServiceSubsystem.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Subsystems/WorldSubsystem.h"
#include "UServerFlowSubsystem.generated.h"

UCLASS()
class SERVERMODULE_API UServerFlowSubsystem : public UWorldSubsystem {
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
	}

	void StartRewardFlow(const FString& PlayerId,const FRewardRequest& Payload,FOnFlowComplete Callback){
		UPlayerValidationSubsystem* PV = GetWorld()->GetSubsystem<UPlayerValidationSubsystem>();
		PV->CheckPlayer(PlayerId,
			FOnPlayerCheck::CreateLambda([this, Payload, Callback](bool bValid, FString Reason)
		{
			URewardServiceSubsystem* RS = GetWorld()->GetSubsystem<URewardServiceSubsystem>();
		}));
	}
};