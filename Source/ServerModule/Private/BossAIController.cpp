#include "BossAIController.h"
#include "AI/UI/BossUIComponent.h"
#include "Shared/AI/BossComponent/BossStateComponent.h"

ABossAIController::ABossAIController() {
	StateComponent = CreateDefaultSubobject<UBossStateComponent>(TEXT("StateComponent"));
	UIComponent = CreateDefaultSubobject<UBossUIComponent>(TEXT("UIComponent"));
}

void ABossAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	RunBehaviorTree( TempBehavior);
	BB = GetBlackboardComponent();
}

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

