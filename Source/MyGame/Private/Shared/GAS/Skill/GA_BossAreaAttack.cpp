#include "Shared/GAS/Skill/GA_BossAreaAttack.h"
#include "GameplayEffect.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "GameFramework/Actor.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayCueManager.h"
#include "Shared/AI/BossCharacter.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"
#include "Data/EGasDataType.h"
#include "Shared/Data/LocalDataBaseLoader.h"
#include "SkillModule/Public/Data/SkillTargetBase.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Utill/UEnumTagMatchHelper.h"
#include "Templates/Casts.h"
#include "Enum/EPlayerState.h"

UGA_BossAreaAttack::UGA_BossAreaAttack(){
    // 기본 속성 설정
    AttackRadius = 500.0f;
    DamageAmount = 100.0f;
    DelayBeforeAttack = 3.0f; // n초 뒤 공격
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

    // 서버 모듈에서 그냥 BP 만 넣어주면 되는줄 알았는데!!!!!
    // 에디터에서 BP를 넣어도 실질적인 데이터는 못찾아와서 메모리 예외가 뜨나보다
    // 서버 모듈 빌드에 포함? 시키면 되는것 같긴 하던데...

    if (!SkillDataAsset) {
        ULocalDataBaseLoader * Loader = NewObject<ULocalDataBaseLoader>(this);
        Loader->Initialize();
        FPrimaryAssetId ID;
        Loader->CheckPrimaryAssetId(115, ID);
        SkillDataAsset = Loader->GetDataFromAssetId<USkillDataAsset>(ID);
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

    // 각 액터에 KnockBackEffect 적용
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
                            //플레이어가 방어 상태일 경우, 작용하지 않음.
                            // 살짝 밀리는 리액션
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