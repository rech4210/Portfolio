#include "AI/BTService_DetectTargets.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Shared/AI/BossCharacter.h"

void UBTService_DetectTargets::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn) return;

	ABossCharacter* Boss = Cast<ABossCharacter>(ControlledPawn);
	if (!Boss) return;
	
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	TArray<AActor*> DetectedTargets = Cast<ABossCharacter>(OwnerComp.GetAIOwner()->GetPawn())->DetectTarget(DetectRadius);
	if (DetectedTargets.Num() == 0) {
		BlackboardComponent->SetValueAsBool(TEXT("IsRangedIn"), false);
		BlackboardComponent->ClearValue(TEXT("TargetActor"));
		return;
	}
	BlackboardComponent->SetValueAsBool(TEXT("IsRangedIn"), true);
	BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), DetectedTargets[0]);
}
