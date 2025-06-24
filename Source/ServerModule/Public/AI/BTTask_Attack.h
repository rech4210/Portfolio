

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Shared/AI/BossCharacter.h"
#include "BTTask_Attack.generated.h"

class UGA_BossAttack;

UCLASS()
class SERVERMODULE_API UBTTask_Attack : public UBTTaskNode {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayAbility> AttackAbility;
	FDelegateHandle EventHandle;
	TObjectPtr<UAbilitySystemComponent> ASC;
protected:
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	void OnAbilityFinished(FGameplayTag EventTag, const FGameplayEventData* EventData);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
private:
	UPROPERTY()
	UBehaviorTreeComponent * BTComponent;
	void CleanUp();
};
