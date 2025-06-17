// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "UServerCoreSubsystem.h"
#include "MyGame/Public/Shared/Utill/FRewardRequest.h"
#include "Subsystems/WorldSubsystem.h"
#include "URewardServiceSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FOnRewardResult, bool /*bSuccess*/, FRewardData /*Data*/);

/**
 * 
 */
UCLASS()
class SERVERMODULE_API URewardServiceSubsystem : public UWorldSubsystem{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		GetWorld()->GetSubsystem<UServerCoreSubsystem>();
			// ->RegisterManager(TEXT("Reward"), this);
	}

	void RequestReward(const FRewardRequest& ReqPayload, FOnRewardResult Callback)
	{
		TSharedRef<IHttpRequest> Req = FHttpModule::Get().CreateRequest();
		// Req->SetURL(GetDefault<URewardServiceConfig>()->ValidateEndpoint);
		Req->SetVerb(TEXT("POST"));
		Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		// Req->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + GetDefault<URewardServiceConfig>()->APIKey);
		// Req->SetContentAsString(ReqPayload.ToJson());
		Req->OnProcessRequestComplete();
			// .BindUObject(this, &URewardServiceSubsystem::OnResponse, Callback);
		Req->ProcessRequest();
	}

private:
	// void OnResponse(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk, FOnRewardResult Callback)
	// {
	// 	bool bSuccess = (bOk && Resp->GetResponseCode() == 200);
	// 	FRewardData Data = bSuccess ? FRewardData::FromJson(Resp->GetContentAsString()) : FRewardData();
	// 	Callback.ExecuteIfBound(bSuccess, Data);
	// }
};