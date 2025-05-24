// GA_Skill1.cpp

#include "Shared/GAS/Skill/GA_Skill1.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTargetActor_Mouse.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Self.h"
#include "Shared/GAS/AbilityTask/GGwaPlayMontageAndWaitForEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Shared/GAS/SkillTargetPolicy/FSkillContext.h"
#include "Shared/Player/GGwaAnimInstance.h"

UGA_Skill1::UGA_Skill1()
{
    AbilityInputID    = EAbilityInputID::Skill1;
    InstancingPolicy    = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy  = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    NetSecurityPolicy   = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Skill1::OnAvatarSet(
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
}

void UGA_Skill1::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* /*TriggerEventData*/)
{

    FScopedPredictionWindow ScopedPredictionWindow(ActorInfo->AbilitySystemComponent.Get());

    if (!CanActivateAbility(Handle, ActorInfo)) {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    if (!ActorInfo || !CommitAbility(Handle, ActorInfo, ActivationInfo)){
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // AvatarActor 유효성 검사
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!IsValid(AvatarActor)){
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 2) 타겟팅 Task
    if (SkillDataAsset->TargetStrategyClass->IsChildOf(USkillTarget_Self::StaticClass())){
        OnTargetDataReceived(FGameplayAbilityTargetDataHandle());
    }
    else
    {
        // 마우스 기반 위치 타겟팅
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
    // 취소 시엔 Ability 종료
    GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Skill1.DirectionPreview")));
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
            FGameplayCueParameters CueParams;
            CueParams.Location = HitPoint;
            CueParams.Normal = HitPoint - GetAvatarActorFromActorInfo()->GetActorLocation(); // 방향성 시각화 지원용
            CueParams.Instigator = GetAvatarActorFromActorInfo()->GetInstigator();
            CueParams.EffectCauser = GetAvatarActorFromActorInfo()->GetInstigator();
            CueParams.RawMagnitude = SkillDataAsset->SkillID;
            GetActorInfo().AbilitySystemComponent->AddGameplayCue(
                FGameplayTag::RequestGameplayTag(FName("GameplayCue.Skill1.DirectionPreview")),
                CueParams
            );
        }
    }


    // 4) 몽타주 Task
    UGGwaPlayMontageAndWaitForEvent* MontageTask =
        UGGwaPlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
            this,
            NAME_None,
            SkillDataAsset->CastMontage,
            FGameplayTagContainer(),
            1.0f,
            NAME_None,
            false
        );

    MontageTask->OnCompleted.AddDynamic(this, &UGA_Skill1::OnMontageCompleted);
    //blend out 문제는 아닌것 같다.
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Skill1::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);

    MontageTask->ReadyForActivation();
}

void UGA_Skill1::OnMontageInterrupted(FGameplayTag /*EventTag*/, FGameplayEventData /*EventData*/)
{
    GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Skill1.DirectionPreview")));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Skill1::OnMontageCompleted(FGameplayTag ,FGameplayEventData){
    if (GetAvatarActorFromActorInfo()->HasAuthority())
    {
        SkillContext = BuildSkillContext(CurrentActorInfo);
        SkillContext.SkillData = SkillDataAsset;
        SkillContext.HitLocation =  HitPoint;
        SkillContext.DetectedActors = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);

        auto* ASC = SkillContext.SourceASC.Get();
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);

        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SkillContext.SkillData->GEClass, 1.f, Context);
        // Spec.Data->SetSetByCallerMagnitude(
        //     FGameplayTag::RequestGameplayTag(SkillAssetTypeTag),
        //     SkillContext.SkillData->SkillID
        // );

        FGameplayEffectContextHandle CoolContext = ASC->MakeEffectContext();
        CoolContext.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);

        FGameplayEffectSpecHandle CoolSpec = ASC->MakeOutgoingSpec(SkillContext.SkillData->CoolTimeGEClass, 1.f, CoolContext);
        CoolSpec.Data->SetSetByCallerMagnitude(
            SkillContext.SkillData->CooldownTag,
            SkillContext.SkillData->CoolTime
        );
        CoolSpec.Data->SetSetByCallerMagnitude(
            FGameplayTag::RequestGameplayTag(SkillAssetTypeTag),
            SkillContext.SkillData->SkillID
        );

        if (SkillContext.SkillData->TargetStrategyClass == USkillTarget_Self::StaticClass()){
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            ASC->ApplyGameplayEffectSpecToSelf(*CoolSpec.Data.Get());
        }
        else{
            //여기서 콜백으로 인해, 버프가 이중 발생. 그러니 쿨타임에만 적용되도록 수정할것.
            for (AActor* Target : SkillContext.DetectedActors){
                if (UGGwaAbilitySystemComponent* TargetASC = GetTargetASC(Target) ){
                    ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
                }
            }
            ASC->ApplyGameplayEffectSpecToSelf(*CoolSpec.Data.Get());
        }
    }

    GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Skill1.DirectionPreview")));

    EndAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo,
        true, 
        false 
    );
}
