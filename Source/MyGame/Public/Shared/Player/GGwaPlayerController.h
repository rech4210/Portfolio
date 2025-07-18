// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/AuthRPCInterface.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "Shared/Utill/FRewardRequest.h"
#include "Shared/Interface/IClientComponentProvider.h"
#include "GGwaPlayerController.generated.h"

class ABossCharacter;
class UBaseDataAsset;
class UAuthSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityDataAssetApplied, UBaseDataAsset*, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDataReceived, const FBossDataStruct&, BossData);

/**
 * Unified Player Controller supporting both server and client functionality
 * Uses interface pattern to avoid circular dependencies with ClientModule
 */
UCLASS(Blueprintable)
class MYGAME_API AGGwaPlayerController : public APlayerController, public IAuthRPCInterface, public IClientComponentProvider {
	GENERATED_BODY()
public:
	AGGwaPlayerController();
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnAbilityDataAssetApplied OnAbilityDataAssetApplied;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnBossDataReceived OnBossDataReceived;

	/* 클라이언트 모듈 UI 설정을 위한 함수*/
	virtual void InitClientWidget(const USkillComponent* SkillComponent);

	UFUNCTION(Client, Reliable)
	virtual void Client_ReceiveBossData(const FBossDataStruct& BossCharacter);
	
	UFUNCTION(Client, Reliable)
	virtual void Client_ReceiveSkillData(const USkillComponent* SkillComponent);

	//클라이언트가 자신에게 Possess한 Pawn을 인식(승인)하도록 알려주는 함수
	virtual void AcknowledgePossession(APawn* PossessedPawn) override;

	UFUNCTION(Server, Reliable)
	void Server_InitiateReward(const FString& PlayerId, const FRewardRequest& Payload);

	

	
	// ============================================================================
	// AUTHENTICATION RPC METHODS
	// ============================================================================

	// RPC Call Interface DI
	virtual void RequestServerRegistration(const FString& Username, const FString& Password) override;
	virtual void RequestServerLogin(const FString& Username, const FString& Password) override;
	virtual void Request_Client_TravelToGameWorld(const FString& MapURL) override;
	virtual bool IsAuthRPCAvailable() const override;
	
	/**
	 * Server RPC: Register new user account
	 * Called from client UI, processed by AuthSubsystem
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Register(const FString& Username, const FString& Password);

	/**
	 * Server RPC: Authenticate user and login
	 * Called from client UI, processed by AuthSubsystem
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Login(const FString& Username, const FString& Password);

	/**
	 * Client RPC: Send registration result back to client
	 */
	UFUNCTION(Client, Reliable)
	void Client_OnRegistrationResult(bool bSuccess, const FString& Message);

	/**
	 * Client RPC: Send login result back to client
	 * If successful, includes token and user ID for client-side storage
	 */
	UFUNCTION(Client, Reliable)
	void Client_OnLoginResult(bool bSuccess, const FString& Token, const FString& UserId);

	/**
	 * Client RPC: Trigger client travel to game world after successful authentication
	 */
	UFUNCTION(Client, Reliable)
	void Client_TravelToGameWorld(const FString& MapURL);

private:
	// ============================================================================
	// AuthSubsystem Event Handlers
	// ============================================================================

	/**
	 * Called when AuthSubsystem completes a registration request
	 */
	UFUNCTION()
	void OnAuthSubsystemRegistrationComplete(bool bSuccess, const FString& Message);

	/**
	 * Called when AuthSubsystem completes an authentication request
	 */
	UFUNCTION()
	void OnAuthSubsystemAuthenticationComplete(bool bSuccess, const FString& Token, const FString& UserId);

private:
	// Note: Client functionality is now handled through interface pattern
	// This avoids circular dependencies between MyGame and ClientModule

	// Client component implementation holder (client builds only)
	UPROPERTY()
	TObjectPtr<UObject> ClientComponentProvider; // Holds reference to client components

	void OnLoginSuccess(const FString& Token);
	void OnLoginFailure(const FString& ErrorReason);

public:
	// ============================================================================
	// IClientComponentProvider INTERFACE IMPLEMENTATION
	// ============================================================================

	// Authentication component interface
	virtual void InitializeClientAuth() override;
	virtual void RequestClientRegistration(const FString& Username, const FString& Password) override;
	virtual void RequestClientLogin(const FString& Username, const FString& Password) override;
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) override;
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) override;

	// UI component interface  
	virtual void InitializeClientUI(const USkillComponent* SkillComponent) override;
	virtual void HandleClientMouseOverDetection() override;
	virtual void NotifyClientStateChanged() override;
	virtual void ReceiveBossDataFromServer(const FBossDataStruct& BossData) override;
	virtual void ReceiveSkillDataFromServer(const USkillComponent* SkillComponent) override;

public:
	// ============================================================================
	// LOGIN UI FUNCTIONALITY (Simplified - delegates to component)
	// ============================================================================

	/**
	 * Request user registration through client component (Client-side)
	 * Called from UI, delegates to ClientAuthComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void RequestRegistration(const FString& Username, const FString& Password);

	/**
	 * Request user login through client component (Client-side) 
	 * Called from UI, delegates to ClientAuthComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void RequestLogin(const FString& Username, const FString& Password);

	/**
	 * Connect to game server with authentication token
	 * Performs ClientTravel with token as URL parameter
	 */
	// UFUNCTION(BlueprintCallable, Category = "Authentication")
	// void ConnectToGameServer(const FString& ServerAddress, const FString& Token);

	// ============================================================================
	// Blueprint Events for UI Integration
	// ============================================================================

	/**
	 * Blueprint event for registration result
	 * Called when server responds to registration request
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Authentication")
	void OnRegistrationResult_BP(bool bSuccess, const FString& Message);

	/**
	 * Blueprint event for login result
	 * Called when server responds to login request
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Authentication")
	void OnLoginResult_BP(bool bSuccess, const FString& Token, const FString& UserId);
};


