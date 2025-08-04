
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Shared/AI/EnemySystemCore/BossStateFlags.h"
#include "Shared/AI/Interface/EnemyDataReceiver.h"
#include "BossAIController.generated.h"

class UBossUIComponent;
class UBossStateComponent;
class UBossBehaviorTree;
class UEnemyAbilitySystemComponent;

UCLASS()
class SERVERMODULE_API ABossAIController : public AAIController, public IEnemyDataReceiver
{
	GENERATED_BODY()

public:
	ABossAIController();
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBossBehaviorTree> BossBehaviorTree;
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBehaviorTree> TempBehavior;
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBlackboardComponent> BB;

	void RequestBossStateChange(EBossState BossState) const;
	virtual void ReceiveEnemyData(FBossDataStruct& Data) override;
private:
	void RequestPhaseChange() const;
	TObjectPtr<UBossStateComponent> StateComponent;
	TObjectPtr<UBossUIComponent> UIComponent;
	FBossDataStruct CachedBossData = {};
};

