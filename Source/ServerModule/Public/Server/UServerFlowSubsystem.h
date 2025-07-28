// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

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
		// Optional: 자기 자신을 Registry에 등록할 수도 있음
	}

	void StartRewardFlow(const FString& PlayerId,
						const FRewardRequest& Payload,
						FOnFlowComplete Callback)
	{
		// 1) 플레이어 검증
		UPlayerValidationSubsystem* PV = GetWorld()->GetSubsystem<UPlayerValidationSubsystem>();
		PV->CheckPlayer(PlayerId,
			FOnPlayerCheck::CreateLambda([this, Payload, Callback](bool bValid, FString Reason)
		{
			// if (!bValid) return Callback.Execute(false, {}, Reason);

			// 2) 보상 요청
			URewardServiceSubsystem* RS = GetWorld()->GetSubsystem<URewardServiceSubsystem>();
		// 	RS->RequestReward(Payload,
		// 		FOnRewardResult::CreateLambda([Callback](bool bOK, FRewardData Data)
		// 	{
		// 		if (!bOK) return Callback.Execute(false, {}, TEXT("Reward Failed"));
		// 		// 3) 추가 서비스 가능...
		// 		Callback.Execute(true, Data, TEXT(""));
		// 	}));
		}));
	}
};