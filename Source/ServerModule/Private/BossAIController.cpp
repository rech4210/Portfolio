
#include "BossAIController.h"
#include "AI/UI/BossUIComponent.h"
#include "Shared/AI/BossComponent/BossStateComponent.h"

/*
 * 1. 보스??BT ?�성
 * 2. Detect기능 ?�공
 * 3. Phase 모듈 ?�성?�것.
 * 4. Boss State, Boss Skill Component ?�성?�것. (AI controller??sence? 기능 ?�용 가?�한가?(
 * 5. 보스??AI Controller(HUD ?�결), Character, ASC, AttributeSet 기본 구조 ?�성 -> Game Mode ?�서 ?�을 ?�성?�켜주도�?
 * 6. 보스???�시 ?�성 ?�치�?PostLogin?�로 ?�음.
 * 7. ?�일책임 ?�칙 기�????�라 모듈 분리, Character???�행 주체 / Controller??로직 FSM ?�주�??�계?�것.
 */

ABossAIController::ABossAIController() {
	StateComponent = CreateDefaultSubobject<UBossStateComponent>(TEXT("StateComponent"));
	UIComponent = CreateDefaultSubobject<UBossUIComponent>(TEXT("UIComponent"));
}

// ?�제 초기??로직
void ABossAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	RunBehaviorTree( TempBehavior);
	BB = GetBlackboardComponent();
}
// Phase 변경에 ?�??책임??가지�??�다. 추후 ?�요?�다�?SRP ?�칙???�라 분리?�것.
void ABossAIController::RequestPhaseChange() const{
	StateComponent->AdvanceBossPhase(BB);
}

void ABossAIController::RequestBossStateChange(EBossState BossState) const{
	StateComponent->UpdateBossState(BB, BossState);
}

void ABossAIController::ReceiveEnemyData(FBossDataStruct& Data){
	if (HasAuthority()) {
		if (Data.Health <70.f) {
			RequestPhaseChange();
		}
	}
}

// void ABossAIController::RequestHUDUpdate(FBossDataStruct& Data) {
// 	UIComponent->UpdateWidgetData(Data);
// }
//

