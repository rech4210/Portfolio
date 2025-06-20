#include "Shared/GAS/Skill/GA_BossAreaAttack.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayCueManager.h"
#include "Shared/AI/BossCharacter.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/GAS/Cue/ECueType.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"
#include "Templates/Casts.h"

UGA_BossAreaAttack::UGA_BossAreaAttack()
{
    // 기본 속성 설정
    AttackRadius = 500.0f;
    DamageAmount = 100.0f;
    DelayBeforeAttack = 3.0f; // n초 뒤 공격
}

void UGA_BossAreaAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData){
    if (ActorInfo->AvatarActor.IsValid()){
        // Gameplay Cue 트리거
        FGameplayCueParameters CueParams;
        CueParams.Instigator = ActorInfo->AvatarActor.Get();
        CueParams.Location = ActorInfo->AvatarActor->GetActorLocation();
        UGGwaAbilitySystemComponent* ASC = Cast<UGGwaAbilitySystemComponent>(Cast<AGGwaPlayerState>(Cast<AGGwaCharacter>(ActorInfo->AvatarActor)->GetPlayerState())->GetAbilitySystemComponent());
        if (ASC){
            //GA -> ASC -> PlayerReactionComponent
            ASC->ExecuteGameplayCueLocal(UEnumTagMatchHelper::GetTagFromEnum(ECueType::AreaAttackWarning), CueParams);
        }

        UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DelayBeforeAttack);
        WaitDelayTask->OnFinish.AddDynamic(this, &UGA_BossAreaAttack::PerformAreaAttack);
        WaitDelayTask->ReadyForActivation();
    }
}

void UGA_BossAreaAttack::PerformAreaAttack(){
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    ABossCharacter* Character = Cast<ABossCharacter>(ActorInfo->AvatarActor);
    if (ActorInfo && Character){
        // 범위 내의 모든 액터를 탐지
        TArray<AActor*> OverlappingActors;
        FCollisionQueryParams QueryParams;
        
        FCollisionShape CollisionShape;
        CollisionShape.SetSphere(AttackRadius);
        QueryParams.AddIgnoredActor(Character);

        TArray<FHitResult> HitResults;
        
        GetWorld()->SweepMultiByChannel(HitResults, Character->GetActorLocation(), Character->GetActorLocation(), FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);

        // Populate OverlappingActors from HitResults
        for (const FHitResult& Hit : HitResults) {
            if (AActor* HitActor = Hit.GetActor()) {
                OverlappingActors.Add(HitActor);
            }
        }

        // 각 액터에 KnockBackEffect 적용
        for (AActor* Actor : OverlappingActors)
        {
            if (Actor && KnockBackEffect)
            {
                FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(KnockBackEffect, GetAbilityLevel());
                if (SpecHandle.IsValid()){
                    ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetTargetASC(Actor));
                }
            }
        }
    }
    EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false);
}