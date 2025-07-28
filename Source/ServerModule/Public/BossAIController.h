
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Shared/AI/EnemySystemCore/BossStateFlags.h"
#include "Shared/AI/EnemySystemCore/BossPhaseTypes.h"
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

	//Never Used... maybe used in the future.
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


/**
 * God 방�? ?�계

 * 1. 컴포?�트 ?�율??강화 (?�임 방식)
 * - 컴포?�트가 로직???��??�으�?처리?�게 ?�임
 * - 컨트롤러??"무엇???��?"�?지?? * - 책임 분리, ?�사?�성 ?? 결합???? */

/**
 * 2. 중재??Mediator) ?�는 ?�사??Facade) ?�턴
 * - ?�러 컴포?�트???�업??조율?�는 중간 계층
 * - 컨트롤러???�일 ?�터?�이?�로 관�?가?? * - 복잡???�호?�용 간소?? ?�장???�리
 */

/**
 * 3. Command/Event ?�턴
 * - ?�동??명령 객체???�벤?�로 추상?? * - ?�행, 지?? 취소 ???�어 ?�연???? * - ?�동 로깅/리플?�이/Undo 기능 ?�장 가?? */

/**
 * - 기본?� 1�??�임 구조�??�작
 * - ?�업 많아지�?2�?Mediator ?�입
 * - 복잡???�션 ?�나 ?�태 기록 ?�요?�면 3�?Command�??�장
 */
