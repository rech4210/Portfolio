#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/AuthRPCInterface.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "Shared/Utill/FRewardRequest.h"
#include "GameSharedModule/Public/Interface/IClientComponentProvider.h"
#include "GameSharedModule/Public/Enums/EClientUIKey.h"
#include "GGwaPlayerController.generated.h"

class IAuthRequestRouter;
class ABossCharacter;
class UBaseDataAsset;
class UAuthSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityDataAssetApplied, UBaseDataAsset*, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDataReceived, const FBossDataStruct&, BossData);
DECLARE_DELEGATE_OneParam(FClientSubsystemDelegate, TScriptInterface<IClientManagerInterface>);


UCLASS(Blueprintable)
class MYGAME_API AGGwaPlayerController : public APlayerController, public IAuthRPCInterface, public IClientManagerInterface, public IAbilitySystemInterface {
	GENERATED_BODY()
public:
	AGGwaPlayerController();
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void AcknowledgePossession(APawn* PossessedPawn) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnAbilityDataAssetApplied OnAbilityDataAssetApplied;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnBossDataReceived OnBossDataReceived;

	UFUNCTION(Server, Reliable)
	void Server_InitiateReward(const FString& PlayerId, const FRewardRequest& Payload);
	
	virtual void RequestServerRegistration(const FString& Username, const FString& Password) override;
	virtual void RequestServerLogin(const FString& Username, const FString& Password) override;
	virtual void Request_Client_TravelToGameWorld(const FString& MapURL) override;
	virtual void Request_Client_ConnectToGameServerWithToken(const FString& Token, const FString& UserId) override;
	virtual bool IsAuthRPCAvailable() const override;
	virtual void NotifyAuthLoginResult(bool bSuccess, const FString& UserId, const FString& Token, const FString& ErrorCode) override;
	virtual void NotifyAuthRegisterResult(bool bSuccess, const FString& UserId, const FString& Token, const FString& ErrorCode) override;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Register(const FString& Username, const FString& Password);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Login(const FString& Username, const FString& Password);
	UFUNCTION(Client, Reliable)
	void Client_TravelToGameWorld(const FString& MapURL);
	
	UPROPERTY(Transient)
	TScriptInterface<IClientManagerInterface> CachedClientManagerInterface;
	
	UPROPERTY(Transient)
	TObjectPtr<UActorComponent> ClientAuthComponent;

	UPROPERTY(Transient)
	TObjectPtr<UActorComponent> ClientUIComponent;

	UFUNCTION(BlueprintCallable, Category = "Client Component")
	UActorComponent* CreateUIComponent(EClientUIKey UIKey);

	UFUNCTION(BlueprintCallable, Category = "Client Component")
	void CreateDefaultClientComponents();
public:
	TScriptInterface<IClientManagerInterface> GetUIManagerInterface();
	virtual void RegistClientComponent(UActorComponent* Component) override;
	
	UFUNCTION(BlueprintCallable, Category = "Client Service")
	virtual void InitializeUI() override;

	UFUNCTION(BlueprintCallable, Category = "Client Service")
	void InitializeClientComponent();
	
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
    virtual void NotifyClientEvent(FGameplayTag Tag) override;
	UFUNCTION(Client, Reliable , BlueprintCallable, Category = "Client Service")
    virtual void ProcessBossData(const FBossDataStruct& BossData) override;

	UFUNCTION(Category = "Client Service")
	virtual void SkillHUDReplication(const FSkillSlotReplicationArray& SkillSlotsReplication) override;

	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void ConnectToGameServerWithToken(const FString& Token, const FString& UserId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Authentication")
	FString GetCachedAuthToken() const { return CachedAuthToken; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Authentication")
	FString GetCachedUserId() const { return CachedUserId; }
	
	UFUNCTION()
	void SetCachedAuthToken(const FString& Token) {CachedAuthToken = Token; }
	UFUNCTION()
	void SetCachedUserId(const FString& Id) {CachedUserId = Id; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
private:
	void TryBindASCInput();
	
	UPROPERTY(Transient)
	FString CachedAuthToken;

	UPROPERTY(Transient)
	FString CachedUserId;
	bool bIsBindComplete = false;
};
