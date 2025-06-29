#pragma once

#include "MyGame/Public/Shared/Mode/BaseGameMode.h"
#include "MyGame/Public/Shared/Mode/ModeType.h"
#include "GameFramework/GameMode.h"
#include "GGwaGameMode.generated.h"

class UAuthVerificationService;
class UBattleFlowController;
class UDatabaseManager;

UCLASS()
class SERVERMODULE_API AGGwaGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AGGwaGameMode();

	void Server_SkillLog(FString Name, const FString& SkillName, FVector SkillLocation);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	void OnCharacterDataLoaded(const TOptional<struct FCharacterData>& CharacterData, APlayerController* NewPlayer);
	virtual void Logout(AController* Exiting) override;
	void OnCharacterDataSaved(bool bSuccess);

protected:
	// BaseGameMode의 훅을 이용해 서버 매니저 초기화
	virtual void InitializeServerManagers() override;

	// 서버 전용 요청 처리
	virtual void RequestFlowControllerInit(EModeType ModeType) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

private:
	UPROPERTY()
	UBattleFlowController* BattleFlowController;
	UPROPERTY()
	TObjectPtr<UAuthVerificationService> AuthVerificationService;
	UPROPERTY()
	TObjectPtr<UDatabaseManager> DatabaseManager;
};