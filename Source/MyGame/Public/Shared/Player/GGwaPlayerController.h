// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/AuthRPCInterface.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "Shared/Utill/FRewardRequest.h"
#include "GGwaPlayerController.generated.h"

class ABossCharacter;
class UBaseDataAsset;
class UAuthSubsystem;

// Forward declarations for client-only classes
#if !UE_SERVER
class AGGwaHUD;
class UGGwaWidget;
class UBossStatusWidget;
#ifdef CLIENTMODULE_API
class UAuthService;
#endif
#endif

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityDataAssetApplied, UBaseDataAsset*, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDataReceived, const FBossDataStruct&, BossData);

/**
 * Unified Player Controller supporting both server and client functionality
 * Uses preprocessor directives to separate client-only code from server builds
 */
UCLASS(Blueprintable)
class MYGAME_API AGGwaPlayerController : public APlayerController, public IAuthRPCInterface {
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

	virtual void NotifyClientStateChanged();

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
	// Note: UI properties are handled through runtime checks rather than UPROPERTY 
	// to avoid server build issues with client-only classes

#if !UE_SERVER
	// Client-only widget classes - not exposed as UPROPERTY to avoid server build issues
	TSubclassOf<UGGwaWidget> WidgetClass;
	TSubclassOf<UBossStatusWidget> BossStatusWidgetClass;
	TObjectPtr<AGGwaHUD> GGwaHUD;

	// hover 대상을 적으로 제어하기 위해, enemy base character 제공할것.
	TWeakObjectPtr<ABossCharacter> LastHoveredEnemy;
#endif

#ifdef CLIENTMODULE_API
	// AuthService is only available in client builds with ClientModule
	TObjectPtr<UAuthService> AuthService;
#endif

	void OnLoginSuccess(const FString& Token);
	void OnLoginFailure(const FString& ErrorReason);

public:
	// ============================================================================
	// LOGIN UI FUNCTIONALITY (Integrated from LoginPlayerController)
	// ============================================================================

	/**
	 * Request user registration through AuthService (Client-side)
	 * Called from UI, delegates to AuthService which uses RPC interface
	 */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void RequestRegistration(const FString& Username, const FString& Password);

	/**
	 * Request user login through AuthService (Client-side) 
	 * Called from UI, delegates to AuthService which uses RPC interface
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

private:
	// ============================================================================
	// AuthService Callbacks (Client-side)
	// ============================================================================

	/**
	 * Called by AuthService when registration completes
	 */
	UFUNCTION()
	void OnRegistrationComplete(bool bSuccess, const FString& Message);

	/**
	 * Called by AuthService when login completes
	 */
	UFUNCTION()
	void OnLoginComplete(bool bSuccess, const FString& Token, const FString& UserId);

private:
};


