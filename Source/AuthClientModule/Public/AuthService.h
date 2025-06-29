#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "AuthService.generated.h"

DECLARE_DELEGATE_OneParam(FLoginSuccessDelegate, const FString& /*Token*/);
DECLARE_DELEGATE_OneParam(FLoginFailureDelegate, const FString& /*ErrorReason*/);

UCLASS(BlueprintType)
class AUTHCLIENTMODULE_API UAuthService : public UObject
{
	GENERATED_BODY()

public:
	UAuthService();

	/**
	 * @brief Attempts to log in to the auth server and retrieve a JWT.
	 * @param UserId The user's ID for authentication.
	 * @param Roles The roles associated with the user.
	 * @param OnSuccess Delegate to call on successful login.
	 * @param OnFailure Delegate to call on failed login.
	 */
	void RequestToken(const FString& UserId, const TArray<FString>& Roles, FLoginSuccessDelegate OnSuccess, FLoginFailureDelegate OnFailure);

private:
	void OnTokenRequestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FLoginSuccessDelegate OnSuccess, FLoginFailureDelegate OnFailure);

	FString AuthServerUrl;
}; 