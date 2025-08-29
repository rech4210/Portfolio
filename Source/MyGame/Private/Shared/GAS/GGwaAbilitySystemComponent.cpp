

#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "GameSharedModule/Public/Data/BuffDataAsset.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"
#include "GameSharedModule/Public/Data/BaseDataAsset.h"
#include "GameSharedModule/Public/Data/EGasDataType.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "MyGame/Public/Shared/Player/Component/PlayerReactionComponent.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"

UGGwaAbilitySystemComponent::UGGwaAbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicated(true);
}

bool UGGwaAbilitySystemComponent::CheckCost(const FGameplayEffectSpecHandle& CostSpecHandle) {
    return true;
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
    if (InstigatorActor){
        PC = Cast<AGGwaPlayerController>(InstigatorActor->GetInstigatorController());
    }
    if (!PC){
        AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(GetOwner());
        PC = PS ? Cast<AGGwaPlayerController>(PS->GetPlayerController()) : nullptr;
    }

    if (!PC || !Spec.Def) return;

    // int32 SkillID = -1;
    // SkillID = Spec.GetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::SkillID), true, SkillID);
    //
    // FPrimaryAssetId AssetId;
    // ULocalDataBaseLoader::CheckPrimaryAssetId(SkillID, AssetId);
    // USkillDataAsset* SkillData = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetId, /*bSync=*/true);
    //     if (!SkillData){
    //         UE_LOG(LogTemp, Warning, TEXT("[ASC] Failed to load SkillData for ID %d"), SkillID);
    //         return;
    //     }
    //     for (UBuffDataAsset* Buff : SkillData->AppliedBuffs) {
    //         OnEffectAssetApplied.Broadcast(Buff);
    //     }

    const FGameplayTagContainer& Tags = Spec.Def->GetGrantedTags();
    if (Tags.HasTag(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::Cooldown))){
        UE_LOG(LogTemp, Warning, TEXT("[ASC] Cooldown Active (%s) [%s]"),
            bIsServer ? TEXT("Server") : TEXT("Client"),
            *Spec.Def.GetName()
        );
    }
}

