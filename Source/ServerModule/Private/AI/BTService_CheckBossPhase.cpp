#include "AI/BTService_CheckBossPhase.h"
#include "BossAIController.h"

void UBTService_CheckBossPhase::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	BossAIController = Cast<ABossAIController>(OwnerComp.GetOwner());
}

void UBTService_CheckBossPhase::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
}


