// @Needmodifi
#include "Shared/GAS/Skill/GA_Skill5.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "SkillModule/Public/Data/SkillTargetActor_Mouse.h"
#include "SkillModule/Public/Data/SkillTarget_Self.h"
#include "Shared/GAS/AbilityTask/GGwaPlayMontageAndWaitForEvent.h"
#include "SkillModule/Public/Data/FSkillContext.h"
#include "Shared/Player/GGwaCharacter.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "GameSharedModule/Public/Data/EGasDataType.h"
#include "GameSharedModule/Public/Enum/ECueType.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"

UGA_Skill5::UGA_Skill5() {
	AbilityInputID = EAbilityInputID::Skill5;
}

void UGA_Skill5::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CheckCooldown(Handle, ActorInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	TWeakObjectPtr<AActor> AvatarActor = ActorInfo ? ActorInfo->AvatarActor : nullptr;
		//
		// if (auto PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController); nullptr != PC) {
		// 	PC->Client_ApplyAbilityDataAsset(BuffDataAsset);
		// 	PC->Client_ApplyAbilityDataAsset(SkillDataAsset);
		// }
	
	if (AvatarActor->HasAuthority()) {
		SkillContext = BuildSkillContext(ActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		SkillContext.DetectedActors = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);
		FGameplayEffectSpecHandle Spec = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, SkillContext.SourceASC->MakeEffectContext());
		Spec.Data->SetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum<EGasDataType>(EGasDataType::Cooldown), SkillDataAsset->CoolTime);
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
