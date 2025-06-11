

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckBossPhase.generated.h"

class ABossAIController;
/**
 * 
 */
UCLASS()
class SERVERMODULE_API UBTService_CheckBossPhase : public UBTService {
	GENERATED_BODY()

public:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TObjectPtr<ABossAIController> BossAIController;
};
