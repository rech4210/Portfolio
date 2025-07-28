// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "UServerCoreSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "Subsystems/WorldSubsystem.h"
#include "UPlayerValidationSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FOnPlayerCheck, bool /*bValid*/, FString /*Reason*/);

/**
 * 
 */
UCLASS()
class SERVERMODULE_API UPlayerValidationSubsystem : public UWorldSubsystem {
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		// GetWorld()->GetSubsystem<UServerCoreSubsystem>();
		// 	->RegisterManager(TEXT("PlayerCheck"), this);
	}

	void CheckPlayer(const FString& PlayerId, FOnPlayerCheck Callback)
	{
		// HTTP 요청 준비 (HTTPS + APIKey)
		TSharedRef<IHttpRequest> Req = FHttpModule::Get().CreateRequest();
		// Req->SetURL(GetDefault<URewardServiceConfig>()->PlayerCheckEndpoint + TEXT("?id=") + PlayerId);
		Req->SetVerb(TEXT("GET"));
		// Req->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + GetDefault<URewardServiceConfig>()->APIKey);
		Req->OnProcessRequestComplete()
			.BindUObject(this, &UPlayerValidationSubsystem::OnResponse, Callback);
		Req->ProcessRequest();
	}

private:
	void OnResponse(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk, FOnPlayerCheck Callback){
		bool bValid = (bOk && Resp->GetResponseCode() == 200);
		FString Reason = bValid ? TEXT("") : Resp->GetContentAsString();
		Callback.ExecuteIfBound(bValid, Reason);
	}
};