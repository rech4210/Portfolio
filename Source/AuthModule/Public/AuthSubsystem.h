#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repository/AuthRepositoryInterface.h"
#include "Interfaces/IHttpRequest.h"
#include "Data/AuthDTO.h"
#include "Domain/AuthDomainService.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
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

private:
	UPROPERTY()
	UAuthRepository* DefaultAuthRepository;

	UPROPERTY()
	TScriptInterface<IAuthRepositoryInterface> AuthRepositoryInterface;

	UPROPERTY()
	UAuthDomainService* DomainService;

	// Node.js Auth Server Configuration
	UPROPERTY(EditAnywhere, Category = "Auth Server")
	FString AuthServerUrl = TEXT("http://127.0.0.1:3000");

	UPROPERTY(EditAnywhere, Category = "Auth Server")
	float RequestTimeoutSeconds = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Auth Server")
	bool bUseExternalAuthServer = true;

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Repository Management (DDD Pattern)
	UFUNCTION(BlueprintPure, Category = "Auth Subsystem")
	TScriptInterface<IAuthRepositoryInterface> GetAuthRepository() const;


	UFUNCTION(BlueprintPure, Category = "Auth Subsystem")
	UAuthDomainService* GetDomainService() const { return DomainService; }

	// ============================================================================
	// Use Case Orchestration - App Layer Responsibilities Only
	// ============================================================================

	/**
	 * Server-side registration request handler
	 * Called from PlayerController RPC
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void RequestServerRegistration(const FString& Username, const FString& Password, const FString& ClientIP);

	/**
	 * Server-side authentication request handler  
	 * Called from PlayerController RPC
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void RequestServerAuthentication(const FString& Username, const FString& Password, const FString& ClientIP, class APlayerController* RequestingController);

	/**
	 * Verify JWT token with external auth server
	 * Used for server-to-server authentication validation
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void VerifyTokenWithAuthServer(const FString& Token, const FString& UserId);

	/**
	 * Administrative functions - Server only
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void AdminLockUserAccount(const FString& UserId, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void AdminUnlockUserAccount(const FString& UserId);

	UFUNCTION(BlueprintCallable, Category = "Auth Subsystem")
	void AdminDeactivateUserAccount(const FString& UserId, const FString& Reason);

	// Events - For broadcasting results to PlayerController
	UPROPERTY(BlueprintAssignable, Category = "Auth Events")
	FOnServerAuthenticationComplete OnServerAuthenticationComplete;

	UPROPERTY(BlueprintAssignable, Category = "Auth Events")
	FOnServerRegistrationComplete OnServerRegistrationComplete;

private:
	// ============================================================================
	// External Auth Server Communication (Node.js)
	// ============================================================================
	
	void SendRegistrationToAuthServer(const FAuthRequestDTO& Request);
	void SendAuthenticationToAuthServer(const FAuthRequestDTO& Request, class APlayerController* RequestingController);
	void SendTokenVerificationToAuthServer(const FString& Token, const FString& UserId);

	// HTTP Response Handlers
	void OnRegistrationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnAuthenticationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, class APlayerController* RequestingController);
	void OnTokenVerificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FString& UserId);

	// ============================================================================
	// Helper Methods
	// ============================================================================
	
	bool ValidateServerAuthority() const;
	void BroadcastAuthenticationResult(bool bSuccess, const FString& Token, const FString& UserId, class APlayerController* TargetController = nullptr);
	void BroadcastRegistrationResult(bool bSuccess, const FString& Message);
	void LogSecurityEvent(const FString& Event, const FString& Details) const;
	FString CreateAuthRequestJson(const FAuthRequestDTO& Request) const;
	bool ParseAuthResponseJson(const FString& ResponseBody, FAuthResponseDTO& OutResponse) const;
	bool ParseDetailedErrorMessage(const FString& ResponseBody, FString& OutErrorMessage);

	// Game Data Loading after successful authentication
	void LoadGameDataForUser(const FString& UserId, class APlayerController* PlayerController);
	void OnGameDataLoaded(bool bSuccess, const FString& UserId, class APlayerController* PlayerController);
};
