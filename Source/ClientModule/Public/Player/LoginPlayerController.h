// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuthClientModule/Public/AuthService.h"
#include "LoginPlayerController.generated.h"

/**
 * Login-specific PlayerController for authentication flow
 * Handles UI and authentication before transitioning to main game
 */
UCLASS()
class CLIENTMODULE_API ALoginPlayerController : public APlayerController 
{
	GENERATED_BODY()

public:
	/**
	 * Request user registration through the authentication system
	 */
	UFUNCTION(BlueprintCallable, Category="Auth")
	void RequestRegistration(const FString& Username, const FString& Password);

	/**
	 * Request user login through the authentication system
	 */
	UFUNCTION(BlueprintCallable, Category="Auth")
	void RequestLogin(const FString& Username, const FString& Password);

	/** Blueprint event: Registration result received */
	UFUNCTION(BlueprintImplementableEvent, Category="Auth")
	void OnRegistrationResult_BP(bool bSuccess, const FString& Message);
	
	/** Blueprint event: Login result received */
	UFUNCTION(BlueprintImplementableEvent, Category="Auth")
	void OnLoginResult_BP(bool bSuccess, const FString& Token, const FString& UserId);

	/** 
	 * Connect to game server and travel to game world
	 * Called after successful authentication 
	 */
	UFUNCTION(BlueprintCallable, Category="Auth")
	void ConnectToGameServer(const FString& ServerAddress, const FString& Token);

protected:
	virtual void BeginPlay() override;

private:
	/** Login UI widget class */
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> LoginWidgetClass;

	/** Created login widget instance */
	UPROPERTY()
	TObjectPtr<UUserWidget> LoginWidgetInstance;

	/** Authentication service */
	UPROPERTY()
	TObjectPtr<UAuthService> AuthService;

	// ============================================================================
	// AuthService Callbacks
	// ============================================================================

	/**
	 * Called when registration request completes
	 */
	void OnRegistrationComplete(bool bSuccess, const FString& Message);

	/**
	 * Called when login request completes
	 */
	void OnLoginComplete(bool bSuccess, const FString& Token, const FString& UserId);
};
