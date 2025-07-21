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
DECLARE_DELEGATE_OneParam(FClientSubsystemDelegate, TScriptInterface<IClientManagerInterface>);

UCLASS(Blueprintable)
class MYGAME_API AGGwaPlayerController : public APlayerController, public IAuthRPCInterface, public IClientManagerInterface {
	GENERATED_BODY()
public:
	AGGwaPlayerController();
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	//클라이언트가 자신에게 Possess한 Pawn을 인식(승인)하도록 알려주는 함수
	virtual void AcknowledgePossession(APawn* PossessedPawn) override;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnAbilityDataAssetApplied OnAbilityDataAssetApplied;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnBossDataReceived OnBossDataReceived;

	FClientSubsystemDelegate OnClientSubsystemDelegate;

	// ============================================================================
	// AUTHENTICATION RPC METHODS
	// ============================================================================
	UFUNCTION(Server, Reliable)
	void Server_InitiateReward(const FString& PlayerId, const FRewardRequest& Payload);
	
	// RPC Call Interface DI From ClientAuthService
	virtual void RequestServerRegistration(const FString& Username, const FString& Password) override;
	virtual void RequestServerLogin(const FString& Username, const FString& Password) override;
	virtual void Request_Client_TravelToGameWorld(const FString& MapURL) override;
	virtual bool IsAuthRPCAvailable() const override;
	

private:
	// RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Register(const FString& Username, const FString& Password);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Login(const FString& Username, const FString& Password);
	UFUNCTION(Client, Reliable)
	void Client_OnRegistrationResult(bool bSuccess, const FString& Message);
	UFUNCTION(Client, Reliable)
	void Client_OnLoginResult(bool bSuccess, const FString& Token, const FString& UserId);
	UFUNCTION(Client, Reliable)
	void Client_TravelToGameWorld(const FString& MapURL);
	
	// ============================================================================
	// AuthSubsystem Event Handlers
	// ============================================================================

	IClientManagerInterface* CachedClientManagerInterface;

	UPROPERTY(EditDefaultsOnly, Category= "Client Component")
	TSubclassOf<UActorComponent> ClientAuthComponentClass;

	UPROPERTY(EditDefaultsOnly, Category= "Client Component")
	TSubclassOf<UActorComponent> ClientUIComponentClass;

	UPROPERTY(EditDefaultsOnly, Category= "Client Component")
	TObjectPtr<UActorComponent> ClientAuthComponent;

	UPROPERTY(EditDefaultsOnly, Category= "Client Component")
	TObjectPtr<UActorComponent> ClientUIComponent;
public:
	// ============================================================================
	// CLIENT SERVICE ACCESS - Interface-based approach
	// ============================================================================

	// Get UIManagerSubsystem interface (cached for performance)
	IClientManagerInterface* GetUIManagerInterface();

	// ============================================================================
	// Subsystem Interface Implementation
	// ============================================================================
	virtual void RegistClientComponent(UActorComponent* Component) override;
	
	UFUNCTION(BlueprintCallable, Category = "Client Service")
	virtual void InitializeUI(const USkillComponent* SkillComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Client Service")
	virtual void ProcessRegistration(const FString& Username, const FString& Password) override;

	UFUNCTION(BlueprintCallable, Category = "Client Service")
	virtual void ProcessLogin(const FString& Username, const FString& Password) override;

	UFUNCTION(BlueprintCallable, Category = "Client Service")
	virtual void HandleRegistrationResult(bool bSuccess, const FString& Message) override;

	UFUNCTION(BlueprintCallable, Category = "Client Service")
	virtual void HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId) override;
	
	UFUNCTION(BlueprintCallable, Category = "Client Service")
    virtual void ProcessMouseOverDetection() override;

	UFUNCTION(BlueprintCallable, Category = "Client Service")
    virtual void NotifyStateChanged() override;
	UFUNCTION(Client, Reliable , BlueprintCallable, Category = "Client Service")
    virtual void ProcessBossData(const FBossDataStruct& BossData) override;
	UFUNCTION(Client, Reliable , BlueprintCallable, Category = "Client Service")
    virtual void ProcessSkillData(const USkillComponent* SkillComponent) override;
	
};


