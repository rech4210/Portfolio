#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"

// Forward declarations
class UAuthService;

#include "ClientAuthComponent.generated.h"

/**
 * UClientAuthComponent
 * 
 * Handles client-specific authentication functionality only.
 * UI management has been separated to UClientUIComponent for better separation of concerns.
 * This component focuses solely on AuthService integration and authentication workflows.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class CLIENTMODULE_API UClientAuthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClientAuthComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// CLIENT AUTHENTICATION INTERFACE
	// ============================================================================

	/**
	 * Initialize AuthService instance for client authentication
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void InitializeAuthService();

	/**
	 * Request user registration through local AuthService
	 * @param Username - User's chosen username
	 * @param Password - User's chosen password  
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void RequestRegistration(const FString& Username, const FString& Password);

	/**
	 * Request user login through local AuthService
	 * @param Username - User's username
	 * @param Password - User's password
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void RequestLogin(const FString& Username, const FString& Password);

	/**
	 * Handle server registration result forwarded from PlayerController
	 * @param bSuccess - Whether registration was successful
	 * @param Message - Success or error message from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void OnServerRegistrationResult(bool bSuccess, const FString& Message);

	/**
	 * Handle server login result forwarded from PlayerController
	 * @param bSuccess - Whether login was successful
	 * @param Token - JWT token if successful
	 * @param UserId - User ID if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId);

	// ============================================================================
	// BLUEPRINT EVENTS (forwarded from PlayerController)
	// ============================================================================

	/**
	 * Blueprint event for registration result
	 * @param bSuccess - Whether registration was successful
	 * @param Message - Success or error message
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Client Auth")
	void OnRegistrationResult_BP(bool bSuccess, const FString& Message);

	/**
	 * Blueprint event for login result
	 * @param bSuccess - Whether login was successful
	 * @param Token - JWT token if successful
	 * @param UserId - User ID if successful
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Client Auth")
	void OnLoginResult_BP(bool bSuccess, const FString& Token, const FString& UserId);

private:
	// ============================================================================
	// PRIVATE AUTHENTICATION MEMBERS
	// ============================================================================

	/**
	 * AuthService instance for client authentication
	 */
	UPROPERTY()
	TObjectPtr<UAuthService> AuthService;

	/**
	 * Owner PlayerController reference
	 */
	UPROPERTY()
	TObjectPtr<class AGGwaPlayerController> OwnerController;

	// ============================================================================
	// AUTHSERVICE CALLBACK HANDLERS
	// ============================================================================

	/**
	 * Handle AuthService registration completion
	 * @param bSuccess - Whether registration was successful
	 * @param Message - Success or error message
	 */
	UFUNCTION()
	void OnRegistrationComplete(bool bSuccess, const FString& Message);

	/**
	 * Handle AuthService login completion
	 * @param bSuccess - Whether login was successful
	 * @param Token - JWT token if successful
	 * @param UserId - User ID if successful
	 */
	UFUNCTION()
	void OnLoginComplete(bool bSuccess, const FString& Token, const FString& UserId);
};
