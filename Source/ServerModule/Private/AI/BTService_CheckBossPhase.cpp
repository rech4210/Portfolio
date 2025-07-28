// @Needmodifi
#include "AI/BTService_CheckBossPhase.h"
#include "BossAIController.h"





//At first, Cast BossComponent and check the phase.
void UBTService_CheckBossPhase::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	BossAIController = Cast<ABossAIController>(OwnerComp.GetOwner());
}

void UBTService_CheckBossPhase::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	//
	// //phase 변경 처리를 컨트롤러로 이관시킬것.
	// BossAIController->RequestPhaseChange(EBossPhase::Phase1);
}


