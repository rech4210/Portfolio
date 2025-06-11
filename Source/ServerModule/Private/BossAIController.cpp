// Fill out your copyright notice in the Description page of Project Settings.

#include "BossAIController.h"
#include "AI/UI/BossUIComponent.h"
#include "Shared/AI/BossComponent/BossStateComponent.h"

/*
 * 1. 보스의 BT 생성
 * 2. Detect기능 제공
 * 3. Phase 모듈 작성할것.
 * 4. Boss State, Boss Skill Component 작성할것. (AI controller의 sence? 기능 사용 가능한가?(
 * 5. 보스의 AI Controller(HUD 연결), Character, ASC, AttributeSet 기본 구조 작성 -> Game Mode 에서 적을 생성시켜주도록.
 * 6. 보스의 임시 생성 위치를 PostLogin으로 잡음.
 * 7. 단일책임 원칙 기준에 따라 모듈 분리, Character는 실행 주체 / Controller는 로직 FSM 위주로 설계할것.
 */

ABossAIController::ABossAIController() {
	StateComponent = CreateDefaultSubobject<UBossStateComponent>(TEXT("StateComponent"));
	UIComponent = CreateDefaultSubobject<UBossUIComponent>(TEXT("UIComponent"));
}

// 실제 초기화 로직
void ABossAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	RunBehaviorTree( TempBehavior);
	BB = GetBlackboardComponent();
}
// 현재 StateComponent가 Phase 변경에 대한 책임도 가지고 있다. 추후 필요하다면 SRP 원칙에 따라 분리할것.
void ABossAIController::RequestPhaseChange() const{
	StateComponent->AdvanceBossPhase(BB);
}

void ABossAIController::RequestBossStateChange(EBossState BossState) const{
	StateComponent->UpdateBossState(BB, BossState);
}

void ABossAIController::ReceiveEnemyData(FBossDataStruct& Data){
	// RequestHUDUpdate(Data);
	if (HasAuthority()) {
		if (Data.Health <70.f) {
			// Enum BossPhase::
			RequestPhaseChange();
		}
	}
}

// void ABossAIController::RequestHUDUpdate(FBossDataStruct& Data) {
// 	UIComponent->UpdateWidgetData(Data);
// }
//

