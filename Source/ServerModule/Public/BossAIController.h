// Fill out your copyright notice in the Description page of Project Settings.

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
 * God 방지 설계

 * 1. 컴포넌트 자율성 강화 (위임 방식)
 * - 컴포넌트가 로직을 내부적으로 처리하게 위임
 * - 컨트롤러는 "무엇을 할지"만 지시
 * - 책임 분리, 재사용성 ↑, 결합도 ↓
 */

/**
 * 2. 중재자(Mediator) 또는 파사드(Facade) 패턴
 * - 여러 컴포넌트의 협업을 조율하는 중간 계층
 * - 컨트롤러는 단일 인터페이스로 관리 가능
 * - 복잡한 상호작용 간소화, 확장에 유리
 */

/**
 * 3. Command/Event 패턴
 * - 행동을 명령 객체나 이벤트로 추상화
 * - 실행, 지연, 취소 등 제어 유연성 ↑
 * - 행동 로깅/리플레이/Undo 기능 확장 가능
 */

/**
 * - 기본은 1번 위임 구조로 시작
 * - 협업 많아지면 2번 Mediator 도입
 * - 복잡한 액션 큐나 상태 기록 필요하면 3번 Command로 확장
 */
