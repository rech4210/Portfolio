#pragma once

#include "MyGame/Public/Shared/Mode/BaseGameMode.h"
#include "MyGame/Public/Shared/Mode/ModeType.h"
#include "GameFramework/GameMode.h"
#include "GameSharedModule/Public/Interface/ISkillRepositoryInterface.h"
#include "Interface/PlayerIdentityInterface.h"
#include "GGwaGameMode.generated.h"

class UAuthVerificationService;
class UBattleFlowController;
class UDatabaseManager;
class IInventoryRepositoryInterface;
class IShopRepositoryInterface;
class AGGwaPlayerState;
class USkillComponent;

USTRUCT()
struct FPlayerIdentityFair {
	GENERATED_BODY()
	FString UserId;
	FString Token;
	bool bIsValid = true;
};

UCLASS()
class SERVERMODULE_API AGGwaGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AGGwaGameMode();
	void Server_SkillLog(FString Name, const FString& SkillName, FVector SkillLocation);

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
private:
	// ============================================================================
	// DDD SYSTEM INITIALIZATION
	// ============================================================================

	void InitializePlayerDDDSystems(APlayerController* NewPlayer, class AGGwaPlayerState* PlayerState, const FString& UserId);

protected:
	virtual void InitializeServerManagers() override;
	
	virtual void RequestFlowControllerInit(EModeType ModeType) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnSkillDataLoadCompleted(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillComponent* SkillComponent);

private:
	UPROPERTY()
	UBattleFlowController* BattleFlowController;
	UPROPERTY()
	TObjectPtr<UAuthVerificationService> AuthVerificationService;
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DatabaseManager;

	TArray<USkillDataAsset*> LoadedSkillDefinitions;

	TMap<FUniqueNetIdRepl, FPlayerIdentityFair> PendingPlayers;

	struct FPendingTokenVerification
	{
		FString Token;
		FString ProvidedUserId;
		FString Address;
		FUniqueNetIdRepl UniqueId;
		double StartTime;
		bool bCompleted;
		bool bSuccess;
		FString ErrorMessage;

		FPendingTokenVerification()
			: StartTime(0.0), bCompleted(false), bSuccess(false) {}
	};

	TMap<FUniqueNetIdRepl, FPendingTokenVerification> PendingTokenVerifications;

	void ProcessPendingTokenVerifications();
	
	FTimerHandle MapTransitionTimer;
};