#pragma once

#include "MyGame/Public/Shared/Mode/BaseGameMode.h"
#include "MyGame/Public/Shared/Mode/ModeType.h"
#include "GameFramework/GameMode.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "GGwaGameMode.generated.h"

class UAuthVerificationService;
class UBattleFlowController;
class UDatabaseManager;
class IInventoryRepositoryInterface;
class IShopRepositoryInterface;
class AGGwaPlayerState;

UCLASS()
class SERVERMODULE_API AGGwaGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AGGwaGameMode();

	virtual void BeginPlay() override;

	void Server_SkillLog(FString Name, const FString& SkillName, FVector SkillLocation);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	void OnCharacterDataLoaded(const TOptional<struct FCharacterData>& CharacterData, APlayerController* NewPlayer);
	virtual void Logout(AController* Exiting) override;
	void OnCharacterDataSaved(bool bSuccess);

private:
	// ============================================================================
	// DDD SYSTEM INITIALIZATION
	// ============================================================================
	
	/**
	 * Initialize all Domain-Driven Design systems for a newly connected player
	 * @param NewPlayer The player controller that just connected
	 * @param PlayerState The player's state object
	 * @param UserId The authenticated user ID from cached credentials
	 */
	void InitializePlayerDDDSystems(APlayerController* NewPlayer, class AGGwaPlayerState* PlayerState, int32 UserId);

protected:
	// BaseGameMode의 훅을 이용해 서버 매니저 초기화
	virtual void InitializeServerManagers() override;

	// 서버 전용 요청 처리
	virtual void RequestFlowControllerInit(EModeType ModeType) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	UBattleFlowController* BattleFlowController;
	UPROPERTY()
	TObjectPtr<UAuthVerificationService> AuthVerificationService;
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DatabaseManager;

	// UPROPERTY()
	// TScriptInterface<IInventoryRepositoryInterface> InventoryRepository;
	// UPROPERTY()
	// TScriptInterface<IInventoryRepositoryInterface> EquipmentRepository;
	// UPROPERTY()
	// TScriptInterface<ISkillRepositoryInterface> SkillRepository;
	// UPROPERTY()
	// TScriptInterface<IShopRepositoryInterface> ShopRepository;

	TArray<USkillDataAsset*> LoadedSkillDefinitions;

	// PreLogin에서 인증에 성공한 유저 정보를 임시 저장하는 맵
	TMap<FUniqueNetIdRepl, FString> PendingPlayers;

	// Async token verification management
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

	// Map to track pending token verifications during PreLogin
	TMap<FUniqueNetIdRepl, FPendingTokenVerification> PendingTokenVerifications;

	// Async token verification callback
	void OnTokenVerificationComplete(bool bSuccess, const FString& VerifiedUserId, const FUniqueNetIdRepl& UniqueId);

	// Check and process completed token verifications
	void ProcessPendingTokenVerifications();
};