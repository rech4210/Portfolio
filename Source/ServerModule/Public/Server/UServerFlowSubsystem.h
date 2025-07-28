
#pragma once

#include "CoreMinimal.h"
#include "UPlayerValidationSubsystem.h"
#include "URewardServiceSubsystem.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Subsystems/WorldSubsystem.h"
#include "UServerFlowSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SERVERMODULE_API UServerFlowSubsystem : public UWorldSubsystem {
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		// Optional: �ڱ� �ڽ��� Registry�� ����� ���� ����
	}

	void StartRewardFlow(const FString& PlayerId,
						const FRewardRequest& Payload,
						FOnFlowComplete Callback)
	{
		// 1) �÷��̾� ����
		UPlayerValidationSubsystem* PV = GetWorld()->GetSubsystem<UPlayerValidationSubsystem>();
		PV->CheckPlayer(PlayerId,
			FOnPlayerCheck::CreateLambda([this, Payload, Callback](bool bValid, FString Reason)
		{
			// if (!bValid) return Callback.Execute(false, {}, Reason);

			// 2) ���� ��û
			URewardServiceSubsystem* RS = GetWorld()->GetSubsystem<URewardServiceSubsystem>();
		// 	RS->RequestReward(Payload,
		// 		FOnRewardResult::CreateLambda([Callback](bool bOK, FRewardData Data)
		// 	{
		// 		if (!bOK) return Callback.Execute(false, {}, TEXT("Reward Failed"));
		// 		// 3) �߰� ���� ����...
		// 		Callback.Execute(true, Data, TEXT(""));
		// 	}));
		}));
	}
};