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
    TargetingStrategy = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass);
    InstancingPolicy    = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy  = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    NetSecurityPolicy   = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Skill1::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData*)
{
    // FScopedPredictionWindow ScopedPredictionWindow(ActorInfo->AbilitySystemComponent.Get());
    if (!CanActivateAbility(Handle, ActorInfo)) {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    if (!ActorInfo || !CommitAbility(Handle, ActorInfo, ActivationInfo)){
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!IsValid(AvatarActor)){
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (SkillDataAsset->TargetStrategyClass->IsChildOf(USkillTarget_Self::StaticClass())){
        OnTargetDataReceived(FGameplayAbilityTargetDataHandle());
    }
    else
    {
        ASkillTargetActor_Mouse* TargetActor =
            NewObject<ASkillTargetActor_Mouse>(this);

        UAbilityTask_WaitTargetData* TargetTask =
            UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
                this,
                FName("Skill1_Target"),
                EGameplayTargetingConfirmation::Instant,
                TargetActor
            );
        TargetTask->ValidData.AddDynamic(this, &UGA_Skill1::OnTargetDataReceived);
        TargetTask->Cancelled.AddDynamic(this, &UGA_Skill1::OnTargetDataCancelled);
        TargetTask->ReadyForActivation();
    }
}

void UGA_Skill1::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_Skill1::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data){
    
    if (!CurrentActorInfo){
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!IsValid(AvatarActor)){
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    if (Data.Num() > 0 && Data.Get(0)){
        const FHitResult* Hit = Data.Get(0)->GetHitResult();
        if (Hit){
            HitPoint = Hit->ImpactPoint;
            FVector Dir = Hit->ImpactPoint - AvatarActor->GetActorLocation();
            AvatarActor->SetActorRotation(Dir.Rotation());
            auto ASC = Cast<UGGwaAbilitySystemComponent>(GetActorInfo().AbilitySystemComponent);
            if (ASC && SkillDataAsset->GE_CueClass) {
                auto Context = ASC->MakeEffectContext();
                Context.AddInstigator(AvatarActor, AvatarActor);
                auto Spec = ASC->MakeOutgoingSpec(SkillDataAsset->GE_CueClass, 1.f, Context);
                Spec.Data->SetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::CueDuration), SkillDataAsset->CueDuration);
                ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
    }
    // UGGwaPlayMontageAndWaitForEvent* MontageTask =
    //     UGGwaPlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
    //         this,
    //         NAME_None,
    //         SkillDataAsset->CastMontage,
    //         FGameplayTagContainer(),
    //         1.0f,
    //         NAME_None,
    //         false
    //     );
    //
    // MontageTask->OnCompleted.AddDynamic(this, &UGA_Skill1::OnMontageCompleted);
    // MontageTask->OnBlendOut.AddDynamic(this, &UGA_Skill1::OnMontageCompleted);
    // MontageTask->OnInterrupted.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);
    // MontageTask->OnCancelled.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);
    //
    // MontageTask->ReadyForActivation();
    
    K2_MontageExectue();
}

void UGA_Skill1::OnMontageInterrupted(FGameplayTag /*EventTag*/, FGameplayEventData /*EventData*/)
{
    GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(UEnumTagMatchHelper::GetTagFromEnum(ECueType::DirectionPreview));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

TArray<AActor*> UGA_Skill1::GetTargetActorsByStrategy(FSkillContext SkillContext) {
    return TargetingStrategy->DetectTargets(SkillContext);
}

void UGA_Skill1::OnMontageCompleted(FGameplayTag ,FGameplayEventData){
    if (GetAvatarActorFromActorInfo()->HasAuthority())
    {
        SkillContext = BuildSkillContext(CurrentActorInfo);
        SkillContext.SkillData = SkillDataAsset;
        SkillContext.HitLocation =  HitPoint;
        
        auto* ASC = SkillContext.SourceASC.Get();
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, Context);
        
        FGameplayEffectContextHandle CoolContext = ASC->MakeEffectContext();
        CoolContext.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);
        FGameplayEffectSpecHandle CoolSpec = ASC->MakeOutgoingSpec(SkillDataAsset->GE_CoolTimeClass, 1.f, CoolContext);
        CoolSpec.Data->SetSetByCallerMagnitude(
            UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::Cooldown),
            SkillDataAsset->CoolTime
        );
        CoolSpec.Data->SetSetByCallerMagnitude(
            SkillAssetTypeTag,
            SkillDataAsset->SkillID
        );

        if (SkillDataAsset->TargetStrategyClass == USkillTarget_Self::StaticClass()){
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            ASC->ApplyGameplayEffectSpecToSelf(*CoolSpec.Data.Get());
        }
        else{
            for (AActor* Target : GetTargetActorsByStrategy(SkillContext)){
                if (UAbilitySystemComponent* TargetASC = GetTargetASC(Target) ){
                    ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
                }
            }
            ASC->ApplyGameplayEffectSpecToSelf(*CoolSpec.Data.Get());
        }
    }

    EndAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo,
        true, 
        false 
    );
}
