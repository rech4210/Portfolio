

#include "Shared/AI/GAS/GA_BossAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "GameplayTags.h"
#include "MyGame/Public/Shared/AI/GAS/AttackProjectile.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "GameSharedModule/Public/Enum/EGasEventType.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"

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
	// FVector TargetLocation = TargetActor->GetActorLocation();
	
	if (HasAuthority(&ActivationInfo)) {
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Instigator = Avatar;
		Params.Owner = Avatar;
		SpawnedProjectile = GetWorld()->SpawnActor<AAttackProjectile>(Projectile, StartLocation, FRotator::ZeroRotator, Params);
		if (SpawnedProjectile) {
			SpawnedProjectile->InitProjectile(StartLocation, TargetActor);
		}
		UE_LOG(LogTemp, Warning, TEXT("Projectile spawned: %s"), *GetNameSafe(SpawnedProjectile));
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetAvatarActorFromActorInfo(),
			UEnumTagMatchHelper::GetTagFromEnum(EGasEventType::AbilityFinished),
			Payload
		);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
