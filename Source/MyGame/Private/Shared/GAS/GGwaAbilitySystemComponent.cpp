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
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/Player/GGwaCharacter.h"

UGGwaAbilitySystemComponent::UGGwaAbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicated(true);
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

    LocalDataBaseLoader = NewObject<ULocalDataBaseLoader>(this);
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
        AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(GetOwner());
        PC = PS ? Cast<AGGwaPlayerController>(PS->GetPlayerController()) : nullptr;
    }

    if (!PC || !Spec.Def) return;

    // SkillID 추출
    int32 SkillID = -1;
    SkillID = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.SkillID"), true, SkillID);
    if (SkillID <= 0){
        UE_LOG(LogTemp, Log, TEXT("[ASC] Invalid SkillID %d"), SkillID);
        return;
    }

    FPrimaryAssetId AssetId;
    LocalDataBaseLoader->GetPrimaryAssetId(SkillID, AssetId);
    USkillDataAsset* SkillData = LocalDataBaseLoader->GetDataFromAssetId<USkillDataAsset>(AssetId, /*bSync=*/true);
    if (!SkillData){
        UE_LOG(LogTemp, Warning, TEXT("[ASC] Failed to load SkillData for ID %d"), SkillID);
        return;
    }

    // 이미 GA의 Cooldown GE 적용 (쿨다운용 따로 만들어줘야하긴 함) 이 되므로 필요없음. 애초에 이게 호출되는 시점에 GE 쿨타임체크가 발동되지않음.
    const FGameplayTagContainer& Tags = Spec.Def->GetGrantedTags();
    if (Tags.HasTag(SkillData->CooldownTag)){
        UE_LOG(LogTemp, Warning, TEXT("[ASC] Cooldown Active (%s) [%s]"),
            bIsServer ? TEXT("Server") : TEXT("Client"),
            *SkillData->DisplayName.ToString()
        );
    }

    //Client RPC를 통한 스킬 ID 전송
    PC->Client_ApplyAbilityDataAsset(SkillData);
    for (const FPrimaryAssetId& BuffId : SkillData->AppliedBuffs)
    {
        UBuffDataAsset* BuffData =
            LocalDataBaseLoader->GetDataFromAssetId<UBuffDataAsset>(BuffId, /*bSync=*/true);
        if (BuffData)
        {
            UE_LOG(LogTemp, Display, TEXT("[ASC] Applied Buff: %s"), *BuffData->DisplayName.ToString());
                PC->Client_ApplyAbilityDataAsset(BuffData);
        }
    }
}

