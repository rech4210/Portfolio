
#pragma once

#include "Shared/Mode/BaseGameMode.h"
#include "Shared/Mode/ModeType.h"
#include "GGwaGameMode.generated.h"

class UBattleFlowController;

UCLASS()
class SERVERMODULE_API AGGwaGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AGGwaGameMode();

	void Server_SkillLog(FString Name, const FString& SkillName, FVector SkillLocation);
protected:
	// BaseGameMode의 훅을 이용해 서버 매니저 초기화
	virtual void InitializeServerManagers() override;

	// 서버 전용 요청 처리
	virtual void RequestFlowControllerInit(EModeType ModeType) override;

private:
	UPROPERTY()
	UBattleFlowController* BattleFlowController;
};