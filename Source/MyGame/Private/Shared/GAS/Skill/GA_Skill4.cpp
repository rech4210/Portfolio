// @Needmodifi
#include "Shared/GAS/Skill/GA_Skill4.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Shared/Player/GGwaCharacter.h"
#include "SkillModule/Public/Data/FSkillContext.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "SkillModule/Public/Data/SkillTargetBase.h"

UGA_Skill4::UGA_Skill4()
{
	AbilityInputID = EAbilityInputID::Skill4;

	// In a real scenario, the SkillDataAsset would be assigned in a Blueprint subclass of this C++ class.
}

void UGA_Skill4::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Cooldown, resource, and other domain checks are now handled by the USkillCastingService
	// before this ability is ever activated. We can proceed directly to the skill's effects.

	if (!SkillDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SkillDataAsset is not set! This should be set in the Blueprint child class."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Failed to commit ability."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s: Activating. This GA now only handles presentation (montages, cues) and the final GE application."), *GetName());

	// --- Main Skill Logic ---
	// This is where the presentation (montage, sounds, particles) and direct gameplay consequences happen.
	// For brevity, we are keeping the core logic of applying the effect to targets.

	if (ActorInfo->IsNetAuthority())
	{
		if (SkillDataAsset->TargetStrategyClass && SkillDataAsset->GEClass)
		{
			// 1. Detect Targets
			SkillContext.SourceActor = GetAvatarActorFromActorInfo();
			SkillContext.SourceASC = GetAbilitySystemComponentFromActorInfo();
			SkillContext.SkillData = SkillDataAsset;

			USkillTargetBase* TargetStrategy = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass);
			const TArray<AActor*> DetectedTargets = TargetStrategy->DetectTargets(SkillContext);

			// 2. Prepare GameplayEffect Spec
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(SkillDataAsset->GEClass, GetAbilityLevel());

			// 3. Apply to all detected targets
			if (EffectSpecHandle.IsValid())
			{
				for (AActor* TargetActor : DetectedTargets)
				{
					if (IAbilitySystemInterface* TargetASCInterface = Cast<IAbilitySystemInterface>(TargetActor))
					{
						SkillContext.SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASCInterface->GetAbilitySystemComponent());
					}
				}
			}
		}
	}

	// The ability's work is done.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
