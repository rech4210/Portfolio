

#include "Server/UServerCoreSubsystem.h"
#include "Server/UServerFlowSubsystem.h"
void UServerCoreSubsystem::InitiateRewardFlow(const FString& PlayerId,
											  const FRewardRequest& Payload,
											  FOnFlowComplete Callback){
	// ���� �帧�� ����ϴ� Subsystem ȣ��
	auto* Flow = GetWorld()->GetSubsystem<UServerFlowSubsystem>();
	Flow->StartRewardFlow(PlayerId, Payload, Callback);
}