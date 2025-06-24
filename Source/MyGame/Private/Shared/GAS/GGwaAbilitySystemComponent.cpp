// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GGwaAbilitySystemComponent.h"
// #include "Shared/Data/BaseDataAsset.h"
#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/ItemDataAsset.h"
#include "Shared/Data/LocalDataBaseLoader.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/Data/BaseDataAsset.h"
#include "Shared/Data/EGasDataType.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/Component/PlayerReactionComponent.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"

UGGwaAbilitySystemComponent::UGGwaAbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicated(true);
    LocalDataBaseLoader = CreateDefaultSubobject<ULocalDataBaseLoader>(TEXT("LocalDataBaseLoader"));
}

void UGGwaAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();
    SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    if (IsOwnerActorAuthoritative())
    {
        OnGameplayEffectAppliedDelegateToSelf.AddUObject(
            this, &UGGwaAbilitySystemComponent::OnGameplayAppliedCallback
        );
    }

    LocalDataBaseLoader->Initialize();

    /**
     *스탯 초기 설정을 위한 GE, CurveTable.
     */
    //ApplyGameplayEffectToSelf(StartupEffect, 1.f, ASC->MakeEffectContext());
}

void UGGwaAbilitySystemComponent::OnGameplayAppliedCallback(
    UAbilitySystemComponent* ASC,
    const FGameplayEffectSpec& Spec,
    FActiveGameplayEffectHandle Handle){
    ProcessGameplayEffect(Spec, /*bIsServer=*/true);
}

void UGGwaAbilitySystemComponent::ProcessGameplayEffect(const FGameplayEffectSpec& Spec,bool bIsServer) const{
    AGGwaPlayerController* PC = nullptr;
    AActor* InstigatorActor = Spec.GetContext().GetOriginalInstigator();
    // Instigator를 통한 시전자 처리, 시전자일경우.
    if (InstigatorActor){
        PC = Cast<AGGwaPlayerController>(InstigatorActor->GetInstigatorController());
    }
    if (!PC){
        //보스 조건부 필요
        AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(GetOwner());
        PC = PS ? Cast<AGGwaPlayerController>(PS->GetPlayerController()) : nullptr;
    }

    if (!PC || !Spec.Def) return;

    // SkillID 추출
    int32 SkillID = -1;
    SkillID = Spec.GetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::SkillID), true, SkillID);
    
    // 해당 로직을 점검..할것
    FPrimaryAssetId AssetId;
    LocalDataBaseLoader->GetPrimaryAssetId(SkillID, AssetId);
    USkillDataAsset* SkillData = LocalDataBaseLoader->GetDataFromAssetId<USkillDataAsset>(AssetId, /*bSync=*/true);
        if (!SkillData){
            UE_LOG(LogTemp, Warning, TEXT("[ASC] Failed to load SkillData for ID %d"), SkillID);
            return;
        }
        for (UBuffDataAsset* Buff : SkillData->AppliedBuffs) {
            // 버프 적용 로직도 구현할것. 현재 SRP 원칙이 지켜지는지 확인.
            // State 적용시 UI나, 이펙트 처리에도 필요할것이라고 판단함.  
            OnEffectAssetApplied.Broadcast(Buff);
        }

    const FGameplayTagContainer& Tags = Spec.Def->GetGrantedTags();
    if (Tags.HasTag(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::Cooldown))){
        UE_LOG(LogTemp, Warning, TEXT("[ASC] Cooldown Active (%s) [%s]"),
            bIsServer ? TEXT("Server") : TEXT("Client"),
            *SkillData->DisplayName.ToString()
        );
    }
}

void UGGwaAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Parameters) {
    // This should only be called on a client.
    // We can get the OwnerActor and check if it's locally controlled.
    AActor* OwnActor = GetOwnerActor();
    if(!OwnActor) return;
    
    APawn* OwnerPawn = Cast<APawn>(OwnActor);
    if(OwnerPawn && OwnerPawn->IsLocallyControlled())
    {
        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwnerActor(), GameplayCueTag, EGameplayCueEvent::Executed, Parameters);
    }
}
