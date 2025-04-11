// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GA_Skill1.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"

UGA_Skill1::UGA_Skill1() {
	AbilityInputID = EAbilityInputID::Skill1;

}

void UGA_Skill1::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	TWeakObjectPtr<AActor> AvatarActor = ActorInfo ? ActorInfo->AvatarActor : nullptr;

	if (!AvatarActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UGA_Skill1A_Skill1: Invalid AvatarActor"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ UGA_Skill1UGA_Skill1 Activated"));

	if (IsLocallyControlled()) {
		if (auto PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController); nullptr != PC) {
			// auto HUD = Cast<>()PC->GetHUD();
			PC->GetDataFromAbility(BuffDataAsset);
			PC->GetDataFromAbility(SkillDataAsset);
		}
	}
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (Spec && Spec->IsActive())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
