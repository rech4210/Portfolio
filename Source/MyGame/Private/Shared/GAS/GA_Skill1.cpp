// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GA_Skill1.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Directional.h"
#include "MyGame/Public/Shared/GAS/SkillTargetPolicy/FSkillContext.h"

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

	// 2. 클라이언트 전용 처리: HUD, UI 반영
	if (IsLocallyControlled()) {
		if (auto PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController); nullptr != PC) {
			// auto HUD = Cast<>()PC->GetHUD();
			PC->GetDataFromAbility(BuffDataAsset);
			PC->GetDataFromAbility(SkillDataAsset);
		}
	}
	
	// 4. 서버 전용 처리: 타격 판정 + GE 적용
	if (AvatarActor->HasAuthority()) {
		SkillContext = BuildSkillContext(ActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		SkillContext.DetectedActors = NewObject<USkillTarget_Directional>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);
		FGameplayEffectSpecHandle Spec = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, SkillContext.SourceASC->MakeEffectContext());
		if (Spec.IsValid()) {
			for (auto& target: SkillContext.DetectedActors)
			{
				if (target->Implements<UAbilitySystemInterface>()){
					auto* TargetASC = Cast<IAbilitySystemInterface>(target)->GetAbilitySystemComponent();
					SkillContext.SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("SkillContext:\n SourceActor: %s\n TargetActor: %s\n TargetLocation: %s\n SkillData: %s\n ComboIndex: %d\n StartTime: %f\n DetectedActors.Num: %d"),
		*GetNameSafe(SkillContext.SourceActor),
		*GetNameSafe(SkillContext.TargetActor),
		*SkillContext.TargetLocation.ToString(),
		*GetNameSafe(SkillContext.SkillData),
		SkillContext.ComboIndex,
		SkillContext.StartTime,
		SkillContext.DetectedActors.Num());
	
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (Spec && Spec->IsActive())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
