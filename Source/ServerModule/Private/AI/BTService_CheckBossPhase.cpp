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
	// //phase ���� ó���� ��Ʈ�ѷ��� �̰���ų��.
	// BossAIController->RequestPhaseChange(EBossPhase::Phase1);
}


