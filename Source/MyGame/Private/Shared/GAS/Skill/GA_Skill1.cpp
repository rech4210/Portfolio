// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Skill/GA_Skill1.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "MyGame/Public/Shared/GAS/SkillTargetPolicy/FSkillContext.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Self.h"

UGA_Skill1::UGA_Skill1() {
	AbilityInputID = EAbilityInputID::Skill1;
}

void UGA_Skill1::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {
	Super::OnAvatarSet(ActorInfo, Spec);
	CooldownGameplayEffectClass = SkillDataAsset->GEClass;
}

void UGA_Skill1::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ActorInfo) {
		return;
	}
	TWeakObjectPtr<AActor> AvatarActor = ActorInfo ? ActorInfo->AvatarActor : nullptr;
	// if (IsLocallyControlled()) {
	// 	if (auto PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController); nullptr != PC) {
	// 		PC->GetDataFromAbility(BuffDataAsset);
	// 		PC->GetDataFromAbility(SkillDataAsset);
	// 	}
	// }
	
	if (AvatarActor->HasAuthority()) {
		SkillContext = BuildSkillContext(ActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		SkillContext.DetectedActors = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);
		FGameplayEffectContextHandle Context = SkillContext.SourceASC->MakeEffectContext();
		UCombinedAbilityDataAsset * CombinedDataAsset = NewObject<UCombinedAbilityDataAsset>();
		CombinedDataAsset->SetData(SkillDataAsset, BuffDataAsset);
		Context.AddSourceObject(CombinedDataAsset);
		FGameplayEffectSpecHandle Spec = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, Context);
		// FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName("Data.Cooldown"), false);
		Spec.Data->SetSetByCallerMagnitude(SkillContext.SkillData->CooldownTag, SkillDataAsset->CoolTime);
		
		//Set in editor
		// ApplyCooldown(Handle, ActorInfo, ActivationInfo);

		// if (!CooldownTag.IsValid())
		// {
		// 	UE_LOG(LogTemp, Error, TEXT("Cooldown Tag not found! Check if it's registered properly."));
		// }
		// else
		// {
		// }
		// if (CooldownGameplayEffectClass == nullptr) {
		// 	CooldownGameplayEffectClass = SkillContext.SkillData->GEClass;
		// 	return;
		// }
		if (Spec.IsValid()) {
			if (auto strategy = Cast<USkillTarget_Self>(SkillContext.SkillData->TargetStrategyClass)) {
				SkillContext.SourceASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
			else if (SkillContext.DetectedActors.Num() > 0)
			{
				for (auto* target : SkillContext.DetectedActors)
				{
					if (UGGwaAbilitySystemComponent* ASC = GetTargetASC(target)) {
						// play monatge UAnimMontage* CastMontage;
						UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
							this,
							NAME_None,
							SkillDataAsset->CastMontage,
							1.0f
						);
						UE_LOG(LogTemp, Warning, TEXT("Task: %s"), *GetNameSafe(Task));
						UE_LOG(LogTemp, Warning, TEXT("Task->Ability: %s"), *GetNameSafe(Task->Ability));
						USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(GetAvatarActorFromActorInfo()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

						//현재 이 compted 부분이 제대로 호출되지 못하는 중.
						// GAS DOC의 기능을 이용하자 시간 너무 허비됨.
						Task->OnCompleted.AddDynamic(this, &UGA_Skill1::OnMontageEnded);
						Task->OnInterrupted.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);
						ASC->AddLooseGameplayTag(SkillDataAsset->UniqueTag);
						Task->ReadyForActivation();
						SkillContext.SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), ASC);
					}
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

void UGA_Skill1::OnMontageEnded() {
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()){
		if (ASC->HasMatchingGameplayTag(SkillDataAsset->UniqueTag)) {
			ASC->RemoveLooseGameplayTag(SkillDataAsset->UniqueTag);
		}
	}
}

void UGA_Skill1::OnMontageInterrupted() {
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()){
		if (ASC->HasMatchingGameplayTag(SkillDataAsset->UniqueTag)) {
			ASC->RemoveLooseGameplayTag(SkillDataAsset->UniqueTag);
			UE_LOG(LogTemp, Warning, TEXT("Montage Interrupted"));
		}
	}
}
