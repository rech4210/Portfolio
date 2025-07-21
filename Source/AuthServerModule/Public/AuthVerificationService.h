#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Net/Core/Connection/NetCloseResult.h"
#include "Json.h"
#include "AuthVerificationService.generated.h"

// Delegate for async token verification result
DECLARE_DELEGATE_TwoParams(FOnTokenVerified, bool /*bSuccess*/, const FString& /*UserId*/);

UCLASS()
class AUTHSERVERMODULE_API UAuthVerificationService : public UObject
{
  GENERATED_BODY()
public:
  /**
   * Verifies a JWT token with the JWT server and extracts the user ID.
   * @param Token The JWT string from the client.
   * @param OutUserId If verification is successful, this will contain the user ID.
   * @return True if the token is valid, false otherwise.
   */
  bool VerifyToken(const FString& Token, FString& OutUserId);

  /**
   * Asynchronous token verification with JWT server (RECOMMENDED)
   * @param Token The JWT token to verify
   * @param OnComplete Callback delegate called when verification completes
   */
  void VerifyTokenAsync(const FString& Token, FOnTokenVerified OnComplete);

  /**
   * Synchronous token verification with JWT server (DEPRECATED - Use VerifyTokenAsync instead)
   * @param Token The JWT token to verify
   * @param OutUserId The extracted user ID if successful
   * @return True if token is valid
   */
  bool VerifyTokenWithServer(const FString& Token, FString& OutUserId);

private:
  // JWT Server configuration
  FString AuthServerUrl = TEXT("http://127.0.0.1:3000");
  float RequestTimeoutSeconds = 10.0f;

  // Async response handler
  void OnTokenVerificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnTokenVerified OnComplete);
};