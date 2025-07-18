#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "MyGame/Public/Shared/Interface/IClientComponentProvider.h"

// Forward declarations
class UAuthService;

#include "ClientAuthComponent.generated.h"

/**
 * UClientAuthComponent
 * 
 * Handles client-specific authentication functionality only.
 * UI management has been separated to UClientUIComponent for better separation of concerns.
 * This component focuses solely on AuthService integration and authentication workflows.
 * Implements IClientAuthInterface for IoC pattern integration.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class CLIENTMODULE_API UClientAuthComponent : public UActorComponent, public IClientAuthInterface
{
	GENERATED_BODY()

public:
	UClientAuthComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// IClientAuthInterface IMPLEMENTATION
	// ============================================================================

	/**
	 * Initialize authentication service (IClientAuthInterface)
	 */
	virtual void InitializeAuth() override;

	/**
	 * Request user registration (IClientAuthInterface)
	 * @param Username - User's chosen username
	 * @param Password - User's chosen password  
	 */
	virtual void RequestRegistration(const FString& Username, const FString& Password) override;

	/**
	 * Request user login (IClientAuthInterface)
	 * @param Username - User's username
	 * @param Password - User's password
	 */
	virtual void RequestLogin(const FString& Username, const FString& Password) override;

	/**
	 * Handle server registration result (IClientAuthInterface)
	 * @param bSuccess - Whether registration was successful
	 * @param Message - Success or error message from server
	 */
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) override;

	/**
	 * Handle server login result (IClientAuthInterface)
	 * @param bSuccess - Whether login was successful
	 * @param Token - JWT token if successful
	 * @param UserId - User ID if successful
	 */
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) override;

	// ============================================================================
	// LEGACY BLUEPRINT INTERFACE (for backward compatibility)
	// ============================================================================

	/**
	 * Legacy wrapper for InitializeAuth
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void InitializeAuthService() { InitializeAuth(); }

	/**
	 * Blueprint callable for registration
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void BP_RequestRegistration(const FString& Username, const FString& Password) { RequestRegistration(Username, Password); }

	/**
	 * Blueprint callable for login
	 */
	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void BP_RequestLogin(const FString& Username, const FString& Password) { RequestLogin(Username, Password); }

	// ============================================================================
	// BLUEPRINT EVENTS (forwarded from PlayerController)
	// ============================================================================

	/**
	 * Blueprint event for registration result
	 * @param bSuccess - Whether registration was successful
	 * @param Message - Success or error message
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Client Auth")
	void BP_OnRegistrationResult(bool bSuccess, const FString& Message);

	/**
	 * Blueprint event for login result
	 * @param bSuccess - Whether login was successful
	 * @param Token - JWT token if successful
	 * @param UserId - User ID if successful
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Client Auth")
	void BP_OnLoginResult(bool bSuccess, const FString& Token, const FString& UserId);

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
	// PRIVATE HELPER METHODS
	// ============================================================================

	/**
	 * Register this component to ClientServiceManager
	 */
	void RegisterSelfToServiceManager();

};
