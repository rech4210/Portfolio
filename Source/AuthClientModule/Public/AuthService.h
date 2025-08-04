#pragma once

#include "CoreMinimal.h"
#include "AuthService.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FRegistrationDelegate, bool, bSuccess, const FString& , Message);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FLoginDelegate, bool, bSuccess, const FString& ,Token, const FString& , UserId);

UCLASS(BlueprintType)
class AUTHCLIENTMODULE_API UAuthService : public UObject
{
	GENERATED_BODY()


public:
	UAuthService();
	UPROPERTY()
	FRegistrationDelegate PendingRegistrationDelegate;
	UPROPERTY()
	FLoginDelegate PendingLoginDelegate;
	
	UFUNCTION(BlueprintCallable, Category = "Auth Service")
	void RequestRegistration(const FString& Username, const FString& Password, APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Auth Service")
	void RequestLogin(const FString& Username, const FString& Password, APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Auth Service")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "Auth Service")
	bool IsAuthenticated() const;

	UFUNCTION(BlueprintPure, Category = "Auth Service")
	FString GetCurrentToken() const;

	UFUNCTION(BlueprintPure, Category = "Auth Service")
	FString GetCurrentUserId() const;

private:
	UPROPERTY()
	FString CurrentToken;

	UPROPERTY()
	FString CurrentUserId;

	UPROPERTY()
	bool bIsAuthenticated = false;
}; 