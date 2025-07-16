#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuthService.generated.h"

// Updated delegates for new authentication flow
DECLARE_DYNAMIC_DELEGATE_TwoParams(FRegistrationDelegate, bool, bSuccess, const FString& , Message);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FLoginDelegate, bool, bSuccess, const FString& ,Token, const FString& , UserId);

/**
 * Client-side Authentication Service
 * Handles UI input and communicates with server through PlayerController RPCs
 * No longer communicates directly with Node.js auth server
 */
UCLASS(BlueprintType)
class AUTHCLIENTMODULE_API UAuthService : public UObject
{
	GENERATED_BODY()

private:
	// Authentication state
	UPROPERTY()
	FString CurrentToken;

	UPROPERTY()
	FString CurrentUserId;

	UPROPERTY()
	bool bIsAuthenticated = false;

	// Pending callbacks for server responses


public:
	UAuthService();
	UPROPERTY()
	FRegistrationDelegate PendingRegistrationDelegate;
	UPROPERTY()
	FLoginDelegate PendingLoginDelegate;
	/**
	 * Request user registration through server
	 * Called from UI, sends request to PlayerController RPC
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Service")
	void RequestRegistration(const FString& Username, const FString& Password, FRegistrationDelegate OnResult);

	/**
	 * Request user login through server
	 * Called from UI, sends request to PlayerController RPC
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Service")
	void RequestLogin(const FString& Username, const FString& Password, FLoginDelegate OnResult);

	/**
	 * Logout current user
	 */
	UFUNCTION(BlueprintCallable, Category = "Auth Service")
	void Logout();

	/**
	 * Check if user is currently authenticated
	 */
	UFUNCTION(BlueprintPure, Category = "Auth Service")
	bool IsAuthenticated() const;

	/**
	 * Get current authentication token
	 */
	UFUNCTION(BlueprintPure, Category = "Auth Service")
	FString GetCurrentToken() const;

	/**
	 * Get current user ID
	 */
	UFUNCTION(BlueprintPure, Category = "Auth Service")
	FString GetCurrentUserId() const;

	// ============================================================================
	// Callbacks from PlayerController (called when server responds)
	// ============================================================================

	/**
	 * Called by PlayerController when server responds to registration request
	 */
	void OnServerRegistrationResult(bool bSuccess, const FString& Message);

	/**
	 * Called by PlayerController when server responds to login request
	 */
	void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId);
}; 