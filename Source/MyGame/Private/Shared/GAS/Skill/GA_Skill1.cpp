#include "Shared/GAS/Skill/GA_Skill1.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "SkillModule/Public/Data/SkillTargetActor_Mouse.h"
#include "SkillModule/Public/Data/SkillTarget_Self.h"
#include "Shared/GAS/AbilityTask/GGwaPlayMontageAndWaitForEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GameSharedModule/Public/Data/EGasDataType.h"
#include "GameSharedModule/Public/Enum/ECueType.h"
#include "SkillModule/Public/Data/FSkillContext.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"

UGA_Skill1::UGA_Skill1()
{
    InstancingPolicy    = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy  = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    NetSecurityPolicy   = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Skill1::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* Data){
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, Data);
}

void UGA_Skill1::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {
    Super::OnAvatarSet(ActorInfo, Spec);
    TargetingStrategy = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass);
}

void UGA_Skill1::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_Skill1::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data){
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    const FHitResult* Hit = Data.Get(0)->GetHitResult();
    if (Hit){
        HitPoint = Hit->ImpactPoint;
        FVector Dir = Hit->ImpactPoint - AvatarActor->GetActorLocation();
        Dir.Z = 0;
        Dir.Normalize();
        AvatarActor->SetActorRotation(Dir.Rotation());
    }
    SkillContext = BuildSkillContext(CurrentActorInfo);
    SkillContext.SkillData = SkillDataAsset;
    SkillContext.HitLocation =  HitPoint;
}

void UGA_Skill1::OnMontageInterrupted(FGameplayTag, FGameplayEventData)
{
    GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(UEnumTagMatchHelper::GetTagFromEnum(ECueType::DirectionPreview));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

TArray<AActor*> UGA_Skill1::GetTargetActorsByStrategy() {
    TargetActors = TargetingStrategy->DetectTargets(SkillContext);
    return TargetActors;
}

void UGA_Skill1::OnMontageCompleted(FGameplayTag Tag ,FGameplayEventData Data){
    if (IsLocallyControlled()) {
        // UI notify
    }
    if (GetAvatarActorFromActorInfo()->HasAuthority())
    {
        auto* ASC = GetAbilitySystemComponentFromActorInfo();

        for (const auto& GEClass : SkillDataAsset->GEClasses)
        {
            if (GEClass)
            {
                FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(GEClass);
                if (SpecHandle.IsValid())
                {
                    for (AActor* Target : TargetActors)
                    {
                        if (UAbilitySystemComponent* TargetASC = GetTargetASC(Target))
                        {
                            ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                        }
                    }
                }
            }
        }
    }
}
