
#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "UServerCoreSubsystem.h"
#include "MyGame/Public/Shared/Utill/FRewardRequest.h"
#include "Subsystems/WorldSubsystem.h"
#include "URewardServiceSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FOnRewardResult, bool /*bSuccess*/, FRewardData /*Data*/);

UCLASS()
class SERVERMODULE_API URewardServiceSubsystem : public UWorldSubsystem{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		GetWorld()->GetSubsystem<UServerCoreSubsystem>();
	}

	void RequestReward(const FRewardRequest& ReqPayload, FOnRewardResult Callback)
	{
		TSharedRef<IHttpRequest> Req = FHttpModule::Get().CreateRequest();
		Req->SetVerb(TEXT("POST"));
		Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Req->OnProcessRequestComplete();
		Req->ProcessRequest();
	}

};