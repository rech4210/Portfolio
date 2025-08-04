#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repository/AuthRepositoryInterface.h"
#include "Interfaces/IHttpRequest.h"
#include "Data/AuthDTO.h"
#include "Domain/AuthDomainService.h"
#include "AuthSubsystem.generated.h"

class UAuthRepository;
class UAuthDomainService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnServerAuthenticationComplete, bool, bSuccess, const FString&, Token, const FString&, UserId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnServerRegistrationComplete, bool, bSuccess, const FString&, Message);

/**
 * Auth Subsystem - Application Layer
 * Orchestrates authentication workflows and manages external service communication
 * Following DDD pattern similar to InventorySubsystem
 */
UCLASS(BlueprintType)
class AUTHMODULE_API UAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	UAuthRepository* DefaultAuthRepository;

	UPROPERTY()
	TScriptInterface<IAuthRepositoryInterface> AuthRepositoryInterface;

	UPROPERTY()
	UAuthDomainService* DomainService;

	UPROPERTY(EditAnywhere, Category = "Auth Server")
	FString AuthServerUrl = TEXT("http://127.0.0.1:3000");

	UPROPERTY(EditAnywhere, Category = "Auth Server")
	float RequestTimeoutSeconds = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Auth Server")
	bool bUseExternalAuthServer = true;

	UPROPERTY()
	TMap<FString, FString> UserTokenCache;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Auth Subsystem")
	TScriptInterface<IAuthRepositoryInterface> GetAuthRepository() const;


	UFUNCTION(BlueprintPure, Category = "Auth Subsystem")
	UAuthDomainService* GetDomainService() const { return DomainService; }

	// ============================================================================
	// Use Case Orchestration - App Layer Responsibilities Only
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void RequestServerRegistration(const FString& Username, const FString& Password, const FString& ClientIP, APlayerController* RequestingController);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void RequestServerAuthentication(const FString& Username, const FString& Password, const FString& ClientIP, class APlayerController* RequestingController);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void VerifyTokenWithAuthServer(const FString& Token, const FString& UserId);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void AdminLockUserAccount(const FString& UserId, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void AdminUnlockUserAccount(const FString& UserId);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void AdminDeactivateUserAccount(const FString& UserId, const FString& Reason);

private:
	// ============================================================================
	// External Auth Server Communication (Node.js)
	// ============================================================================
	
	void SendRegistrationToAuthServer(const FAuthRequestDTO& Request, APlayerController* RequestingController);
	void SendAuthenticationToAuthServer(const FAuthRequestDTO& Request, class APlayerController* RequestingController);
	void SendTokenVerificationToAuthServer(const FString& Token, const FString& UserId);

	void OnRegistrationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, APlayerController* RequestingController);
	void OnAuthenticationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, class APlayerController* RequestingController);
	void OnTokenVerificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FString& UserId);

	// ============================================================================
	// Helper Methods
	// ============================================================================
	
	bool ValidateServerAuthority() const;

	void LogSecurityEvent(const FString& Event, const FString& Details) const;
	FString CreateAuthRequestJson(const FAuthRequestDTO& Request) const;
	bool ParseAuthResponseJson(const FString& ResponseBody, FAuthResponseDTO& OutResponse) const;
	bool ParseDetailedErrorMessage(const FString& ResponseBody, FString& OutErrorMessage);

	void RequestConnectingServer(bool bSuccess, const FString& UserId, class APlayerController* PlayerController);
	
	void CacheTokenForUser(const FString& UserId, const FString& Token);
	FString GetCachedTokenForUser(const FString& UserId) const;
	void ClearTokenCache();
};
