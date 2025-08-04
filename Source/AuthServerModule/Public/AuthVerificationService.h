#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Net/Core/Connection/NetCloseResult.h"
#include "AuthVerificationService.generated.h"

DECLARE_DELEGATE_TwoParams(FOnTokenVerified, bool /*bSuccess*/, const FString& /*UserId*/);

UCLASS()
class AUTHSERVERMODULE_API UAuthVerificationService : public UObject
{
  GENERATED_BODY()
public:
  bool VerifyToken(const FString& Token, FString& OutUserId);

  void VerifyTokenAsync(const FString& Token, FOnTokenVerified OnComplete);

  bool VerifyTokenWithServer(const FString& Token, FString& OutUserId);

private:
  FString AuthServerUrl = TEXT("http://127.0.0.1:3000");
  float RequestTimeoutSeconds = 10.0f;

  void OnTokenVerificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnTokenVerified OnComplete);
};