 #pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/Object.h"
#include "AuthExternalService.generated.h"

 DECLARE_DELEGATE_ThreeParams(FOnAuthLoginResult, bool /*bSuccess*/, const FString& /*UserId*/, const FString& /*Token*/);
 DECLARE_DELEGATE_TwoParams(FOnAuthRegisterResult, bool /*bSuccess*/, const FString& /*UserId*/);
 DECLARE_DELEGATE_TwoParams(FOnAuthVerifyResult, bool /*bValid*/, const FString& /*UserId*/);

 /* Lightweight HTTP adapter to external auth server */
 UCLASS()
 class AUTHSERVERMODULE_API UAuthExternalService : public UObject
 {
 	GENERATED_BODY()
 public:
 	void Initialize(const FString& InBaseUrl, float InTimeoutSeconds = 10.f);

 	// Dynamic configuration reload (optional)
 	void Configure(const FString& InBaseUrl, float InTimeoutSeconds);

 	// Async operations
 	void LoginAsync(const FString& Username, const FString& Password, const FString& ClientIP, FOnAuthLoginResult Callback);
 	void RegisterAsync(const FString& Username, const FString& Password, const FString& ClientIP, FOnAuthRegisterResult Callback);
 	void VerifyTokenAsync(const FString& Token, FOnAuthVerifyResult Callback);

 	// Accessors
 	const FString& GetBaseUrl() const { return BaseUrl; }
 	float GetTimeoutSeconds() const { return TimeoutSeconds; }

 private:
 	FString BaseUrl;
 	float TimeoutSeconds = 10.f;

 	void OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnAuthLoginResult Callback);
 	void OnRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnAuthRegisterResult Callback);
 	void OnVerifyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnAuthVerifyResult Callback);
 };
