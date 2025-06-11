#include "AI/BTTask_Attack.h"
#include "MyGame/Public/Shared/AI/GAS/GA_BossAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Shared/AI/BossCharacter.h"
#include "GameplayTags.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!AttackAbility)
		return EBTNodeResult::Failed;

	ASC = Cast<ABossCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("SelfActor")))->GetAbilitySystemComponent();
	if (!ASC)
		return EBTNodeResult::Failed;
	FGameplayTag FinishTag = FGameplayTag::RequestGameplayTag(TEXT("Event.GA.Finished"));
	FGameplayTagContainer TagContainer(FinishTag);
	EventHandle = ASC->AddGameplayEventTagContainerDelegate(
		TagContainer,
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UBTTask_Attack::OnFinished)
	);
	
	bFinished = false;
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AttackAbility);
	if (!Spec) {
		return EBTNodeResult::Failed;
	}
	bool bActivated = ASC->TryActivateAbility(Spec->Handle, false);
	if (!bActivated) {
		UE_LOG(LogTemp, Warning, TEXT("UBTTask_Attack::ExecuteTask: TryActivateAbility failed for %s"), *AttackAbility->GetName());
		return EBTNodeResult::Failed;
	}
	bNotifyTick = true;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnFinished(FGameplayTag EventTag, const FGameplayEventData* EventData) {
	bFinished = true;
}

// 종료 조건이 잘 이루어지지 않음.
void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	if (!bFinished) return;

	FGameplayTag FinishTag = FGameplayTag::RequestGameplayTag(TEXT("Event.GA.Finished"));
	FGameplayTagContainer TagContainer(FinishTag);
	if (ASC && FinishTag.IsValid() && EventHandle.IsValid()) {
		ASC->RemoveGameplayEventTagContainerDelegate(TagContainer,EventHandle);
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
