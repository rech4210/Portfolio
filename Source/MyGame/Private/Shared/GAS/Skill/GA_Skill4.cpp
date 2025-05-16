#include "Shared/GAS/Skill/GA_Skill4.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Directional.h"
#include "MyGame/Public/Shared/GAS/SkillTargetPolicy/FSkillContext.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"

UGA_Skill4::UGA_Skill4() {
	AbilityInputID = EAbilityInputID::Skill4;
}

void UGA_Skill4::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CheckCooldown(Handle, ActorInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	TWeakObjectPtr<AActor> AvatarActor = ActorInfo ? ActorInfo->AvatarActor : nullptr;

		if (auto PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController); nullptr != PC) {
			PC->Client_ApplyAbilityDataAsset(BuffDataAsset);
			PC->Client_ApplyAbilityDataAsset(SkillDataAsset);
		}
	
	if (AvatarActor->HasAuthority()) {
		SkillContext = BuildSkillContext(ActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		SkillContext.DetectedActors = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);
		FGameplayEffectSpecHandle Spec = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, SkillContext.SourceASC->MakeEffectContext());
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Cooldown"), SkillDataAsset->CoolTime);
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
