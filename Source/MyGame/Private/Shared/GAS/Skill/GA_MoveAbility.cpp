
#include "Shared/GAS/Skill/GA_MoveAbility.h"

#include "AbilitySystemComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaCharacter.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "SkillModule/Public/Data/SkillTargetActor_Mouse.h"
#include "Utill/UEnumTagMatchHelper.h"


UGA_MoveAbility::UGA_MoveAbility() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_MoveAbility::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	AGGwaCharacter* Avatar = Cast<AGGwaCharacter>(Info->AvatarActor.Get());
	AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(Info->PlayerController);

	if (!Avatar || !PC || Data.Num() == 0) return;

	FVector_NetQuantize Point = Data.Get(0)->GetHitResult()->ImpactPoint;
	FRotator HitRotation = FRotator::ZeroRotator;

	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), Avatar->GetActorLocation(), Point);
	if (Avatar->IsLocallyControlled()) {
		GetWorld()->SpawnActor<AActor>(LineTraceRing, Point, HitRotation);
	}
	if (Avatar->HasAuthority()) {
		Avatar->SetMoveData(Path->PathPoints, 1, true);
	}


	// if (Path && Path->PathPoints.Num() > 1) {
	// 	if (Avatar->IsLocallyControlled()) {
	// 		for (int32 i = 0; i < Path->PathPoints.Num() - 1; ++i)
	// 		{
	// 			DrawDebugLine(GetWorld(), Path->PathPoints[i], Path->PathPoints[i + 1], FColor::Green, false, 5.0f, 0, 5.0f);
	// 			DrawDebugSphere(GetWorld(), Path->PathPoints[i], 20.0f, 12, FColor::Red, false, 5.0f);
	// 		}
	// 	}
	// }

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MoveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
									  const FGameplayAbilityActorInfo* ActorInfo,
									  const FGameplayAbilityActivationInfo ActivationInfo,
									  const FGameplayEventData* TriggerEventData){
	if (!CanActivateAbility(Handle, ActorInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
    
	if (!ActorInfo || !CommitAbility(Handle, ActorInfo, ActivationInfo)){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitTargetData* Task = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
		this,
		"SkillTargeting",
		EGameplayTargetingConfirmation::Instant,
		NewObject<ASkillTargetActor_Mouse>(this)
	);

	if (CooldownGameplayEffectClass)
	{
		FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass);
		CooldownSpecHandle.Data->SetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::Cooldown), CoolTime);
		CooldownSpecHandle.Data->DynamicGrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Data.Cooldown.Move")));
		ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CooldownSpecHandle);
	}
	
	if (Task)
	{
		Task->ValidData.AddDynamic(this, &UGA_MoveAbility::OnTargetDataReceived);
		Task->ReadyForActivation();
	}
}

