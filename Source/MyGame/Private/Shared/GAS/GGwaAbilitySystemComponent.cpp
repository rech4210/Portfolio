// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GGwaAbilitySystemComponent.h"
// #include "Shared/Data/BaseDataAsset.h"
#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/CombinedAbilityDataAsset.h"
#include "Shared/Data/ItemDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"

UGGwaAbilitySystemComponent::UGGwaAbilitySystemComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UGGwaAbilitySystemComponent::BeginPlay() {
	Super::BeginPlay();
	/** Called on server whenever a GE is applied to self. This includes instant and duration based GEs. */
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UGGwaAbilitySystemComponent::OnGameplayAppliedCallback);
	
}

void UGGwaAbilitySystemComponent::OnGameplayAppliedCallback(UAbilitySystemComponent* ASC,
	const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle) {

	if (AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(GetOwner()))
	{
		if (AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(PS->GetPlayerController()))
		{
			if (PC->IsLocalController())
			{
				if (UCombinedAbilityDataAsset* Data = Cast<UCombinedAbilityDataAsset>(Spec.GetContext().GetSourceObject()))
				{
					const FGameplayTagContainer& Container = Spec.Def->GetGrantedTags();
					if (Container.HasTag(Data->GetData<USkillDataAsset>()->UniqueTag))
					{
						// 쿨타임 진행중
						UE_LOG(LogTemp, Warning, TEXT("Cooldown %s"), *Data->GetName());
					}
					else
					{
						if (USkillDataAsset* skill = Data->GetData<USkillDataAsset>())
							PC->GetDataFromAbility(skill);
						if (UBuffDataAsset* buff = Data->GetData<UBuffDataAsset>())
							PC->GetDataFromAbility(buff);
						if (UItemDataAsset* item = Data->GetData<UItemDataAsset>())
							PC->GetDataFromAbility(item);
					}
				}
			}
			else if (PC->HasAuthority()) {
				UE_LOG(LogTemp, Warning, TEXT("Server GE Activated %s"), *Spec.GetContext().GetEffectCauser()->GetName());
			}
		}
	}
}

