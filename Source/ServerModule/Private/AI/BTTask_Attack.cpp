#include "AI/BTTask_Attack.h"
#include "MyGame/Public/Shared/AI/GAS/GA_BossAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Shared/AI/BossCharacter.h"
#include "GameplayTags.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"
#include "Shared/GAS/EGasEventType.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BTComponent = &OwnerComp;

	if (!AttackAbility)
	{
		return EBTNodeResult::Failed;
	}

	auto* SelfActor = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("SelfActor"));
	ABossCharacter* BossCharacter = Cast<ABossCharacter>(SelfActor);
	if (!BossCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTTask_Attack::ExecuteTask: SelfActor is not a BossCharacter."));
		return EBTNodeResult::Failed;
	}

	ASC = BossCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTTask_Attack::ExecuteTask: Could not get AbilitySystemComponent."));
		return EBTNodeResult::Failed;
	}

	
	const FGameplayTag FinishEventTag = FGameplayTag::RequestGameplayTag("GasEvent.AbilityFinished");
	if (!FinishEventTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UBTTask_Attack::ExecuteTask: Invalid GameplayTag for AbilityFinished."));
		return EBTNodeResult::Failed;
	}

	EventHandle = ASC->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(FinishEventTag),
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UBTTask_Attack::OnAbilityFinished)
	);

	// 현재 boss area 공격의 경우, OnAbilityFinished 가 제대로 적용되지 않는 경우가 존재함.
	// Area attack 이 실행되고 난 후, task가 종료되지 못해서 무한정 대기중임.
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AttackAbility);
	if (!Spec)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTTask_Attack::ExecuteTask: Ability spec not found for %s"), *AttackAbility->GetName());
		CleanUp();
		return EBTNodeResult::Failed;
	}
	
	const bool bActivated = ASC->TryActivateAbility(Spec->Handle, false);
	if (!bActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTTask_Attack::ExecuteTask: TryActivateAbility failed for %s"), *AttackAbility->GetName());
		CleanUp();
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnAbilityFinished(FGameplayTag EventTag, const FGameplayEventData* EventData)
{
	if (BTComponent)
	{
		FinishLatentTask(*BTComponent, EBTNodeResult::Succeeded);
	}
	CleanUp();
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CleanUp();
	return EBTNodeResult::Aborted;
}

void UBTTask_Attack::CleanUp()
{
	if (ASC && EventHandle.IsValid())
	{
		const FGameplayTag FinishEventTag = FGameplayTag::RequestGameplayTag("GasEvent.AbilityFinished");
		if (FinishEventTag.IsValid())
		{
			ASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(FinishEventTag), EventHandle);
		}
		EventHandle.Reset();
	}
	ASC = nullptr;
	BTComponent = nullptr;
}
