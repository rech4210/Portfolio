// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/AI/GAS/GA_BossAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "GameplayTags.h"
#include "Shared/AI/GAS/AttackProjectile.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Shared/AI/BossCharacter.h"

void UGA_BossAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	ABossCharacter* Avatar = Cast<ABossCharacter>(ActorInfo->AvatarActor.Get());
	AAIController* Controller = Cast<AAIController>(ActorInfo->OwnerActor->GetInstigatorController());
	if (!Avatar || !Projectile || !Controller){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
		
	AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(FName("TargetActor")));
	if (!TargetActor) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	FVector StartLocation = Avatar->GetActorLocation() + ProjectileOffset;
	FVector TargetLocation = TargetActor->GetActorLocation();
	
	if (HasAuthority(&ActivationInfo)) {
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Instigator = Avatar;
		Params.Owner = Avatar;
		SpawnedProjectile = GetWorld()->SpawnActor<AAttackProjectile>(Projectile, StartLocation, FRotator::ZeroRotator, Params);
		if (SpawnedProjectile) {
			SpawnedProjectile->InitProjectile(StartLocation, TargetLocation);
		}
		UE_LOG(LogTemp, Warning, TEXT("Projectile spawned: %s"), *GetNameSafe(SpawnedProjectile));
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetAvatarActorFromActorInfo(),
			FGameplayTag::RequestGameplayTag(TEXT("Event.GA.Finished")),
			Payload
		);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
