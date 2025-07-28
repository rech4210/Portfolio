// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/UServerCoreSubsystem.h"
#include "Server/UServerFlowSubsystem.h"
void UServerCoreSubsystem::InitiateRewardFlow(const FString& PlayerId,
											  const FRewardRequest& Payload,
											  FOnFlowComplete Callback){
	// 실제 흐름을 담당하는 Subsystem 호출
	auto* Flow = GetWorld()->GetSubsystem<UServerFlowSubsystem>();
	Flow->StartRewardFlow(PlayerId, Payload, Callback);
}