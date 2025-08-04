
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_DetectTargets.generated.h"

class ABossCharacter;
UCLASS()
class SERVERMODULE_API UBTService_DetectTargets : public UBTService
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="BTService")
	float DetectRadius = 500.0f;

private:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
