#include "Shared/GAS/Skill/GA_BossAreaAttack.h"
#include "GameplayEffect.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "GameFramework/Actor.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayCueManager.h"
#include "Shared/AI/BossCharacter.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"
#include "Data/EGasDataType.h"
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"
#include "SkillModule/Public/Data/SkillTargetBase.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Utill/UEnumTagMatchHelper.h"
#include "Templates/Casts.h"
#include "Enum/EPlayerState.h"

UGA_BossAreaAttack::UGA_BossAreaAttack(){
    // 기본 ?�성 ?�정
    AttackRadius = 500.0f;
    DamageAmount = 100.0f;
    DelayBeforeAttack = 3.0f; // n�???공격
}

void UGA_BossAreaAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData){
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    if (!HasAuthority(&ActivationInfo) || !SkillDataAsset)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ABossCharacter* BossCharacter = Cast<ABossCharacter>(ActorInfo->AvatarActor.Get());
    if (!BossCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("GA_BossAreaAttack failed: AvatarActor is not a BossCharacter."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UEnemyAbilitySystemComponent* E_ASC = Cast<UEnemyAbilitySystemComponent>(BossCharacter->GetAbilitySystemComponent());
    if (!E_ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("GA_BossAreaAttack failed: AbilitySystemComponent is not UEnemyAbilitySystemComponent."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Static ??공유?�는 helper???�언�??�의??링커 ?�존?�이 ?�다???�을 명심?�것.
    // ?�버 모듈?�서 ?��??�에 ?�킬 ?�이??로드
    if (!SkillDataAsset) {
        FPrimaryAssetId ID;
        if (ULocalDataBaseLoader::CheckPrimaryAssetId(115, ID)) {
            SkillDataAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(ID);
        }
    }
    FGameplayTag test = UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::CueDuration);
    
    UE_LOG(LogTemp, Log, TEXT("GA_BossAreaAttack::ActivateAbility: CueDuration Tag is %s"), *test.ToString());
    
    if (SkillDataAsset)
    {
        FGameplayEffectContextHandle EffectContext = E_ASC->MakeEffectContext();
        EffectContext.AddInstigator(BossCharacter, BossCharacter);

        FGameplayEffectSpecHandle SpecHandle = E_ASC->MakeOutgoingSpec(SkillDataAsset->GE_CueClass, 1.0f, EffectContext);
        if (SpecHandle.IsValid())
        {
            FGameplayTag Tag = UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::CueDuration);
            // FGameplayTag::RequestGameplayTag("Data.CueDuration"), SkillDataAsset->CueDuration
            SpecHandle.Data->SetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::CueDuration), SkillDataAsset->CueDuration);
            E_ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MakeOutgoingSpec failed for GE_CueClass: %s"), *GetNameSafe(SkillDataAsset->GE_CueClass));
        }
    }
    UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DelayBeforeAttack);
    WaitDelayTask->OnFinish.AddDynamic(this, &UGA_BossAreaAttack::PerformAreaAttack);
    WaitDelayTask->ReadyForActivation();
}

void UGA_BossAreaAttack::PerformAreaAttack(){
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
        return;
    }
    
    ABossCharacter* Character = Cast<ABossCharacter>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }

    SkillContext = BuildSkillContext(GetCurrentActorInfo());
    SkillContext.SourceASC = Cast<UEnemyAbilitySystemComponent>(ActorInfo->AbilitySystemComponent);
    if (!SkillContext.SourceASC)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }

    USkillTargetBase* TargetDetector = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass);
    if (!TargetDetector)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }
    
    SkillContext.SkillData = SkillDataAsset;
    SkillContext.DetectedActors = TargetDetector->DetectTargets(SkillContext);

    // �??�터??KnockBackEffect ?�용
    if (SkillDataAsset->GEClass)
    {
        for (AActor* TargetActor : SkillContext.DetectedActors)
        {
            if (TargetActor)
            {
                FGameplayEffectSpecHandle SpecHandle = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.0f, SkillContext.SourceASC->MakeEffectContext());
                if (SpecHandle.IsValid())
                {
                    
                    if (auto ASC = GetTargetASC(TargetActor)) {
                        if (ASC->HasMatchingGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Guard))) {
                            //?�레?�어가 방어 ?�태??경우, ?�용?��? ?�음.
                            // ?�짝 밀리는 리액??
                            return;
                        }
                        
                        if (!TagEffect)
                        {
                            UE_LOG(LogTemp, Error, TEXT("UKnockBackExecution: TagEffect is not set!"));
                            return;
                        }

                        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(TagEffect, 1.f, FGameplayEffectContextHandle());
                        if(!Spec.IsValid())
                        {
                            UE_LOG(LogTemp, Error, TEXT("UKnockBackExecution: Failed to create a valid spec from KnockBackEffectClass!"));
                            return;
                        }
	
                        const FGameplayTag DurationTag = UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::TagDuration);
                        UE_LOG(LogTemp, Log, TEXT("UKnockBackExecution: Applying knockback tag with duration %f, using tag %s"), KnockbackTagMagnitude, *DurationTag.ToString());
	
                        SkillContext.SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
                        
                        Spec.Data->SetSetByCallerMagnitude(DurationTag, KnockbackTagMagnitude);
                        ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                    }
                }
            }
        }
    }
    
    EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false);
}