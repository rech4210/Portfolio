

#include "Server/UServerCoreSubsystem.h"
#include "Server/UServerFlowSubsystem.h"
void UServerCoreSubsystem::InitiateRewardFlow(const FString& PlayerId,
											  const FRewardRequest& Payload,
											  FOnFlowComplete Callback){
	auto* Flow = GetWorld()->GetSubsystem<UServerFlowSubsystem>();
	Flow->StartRewardFlow(PlayerId, Payload, Callback);
}