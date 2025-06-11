

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Shared/AI/BossCharacter.h"
#include "BTTask_Attack.generated.h"

class UGA_BossAttack;
/**
 * 
 */
UCLASS()
class SERVERMODULE_API UBTTask_Attack : public UBTTaskNode {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGA_BossAttack> AttackAbility;
	bool bFinished = false;
	FDelegateHandle EventHandle;
	TObjectPtr<UAbilitySystemComponent> ASC;
	// UPROPERTY(EditAnywhere, Category= "Offset")
	// FVector ATTACK_OFFSET;
	// UPROPERTY(EditAnywhere, Category= "Actor")
	// TSubclassOf<AActor> AttackActor;
	//
	// UPROPERTY()
	// AActor* SpawnedActor;
	// UPROPERTY(VisibleAnywhere)
	// FVector SpawnLocation;
	// UPROPERTY(VisibleAnywhere)
	// FVector TargetLocation;
	// float Alpha = 0.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	void OnFinished(FGameplayTag EventTag, const FGameplayEventData* EventData);
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
