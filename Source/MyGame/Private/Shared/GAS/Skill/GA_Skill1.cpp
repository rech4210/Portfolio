// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Skill/GA_Skill1.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "MyGame/Public/Shared/GAS/SkillTargetPolicy/FSkillContext.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Engine/AssetManager.h"

#include "Shared/Data/BuffDataAsset.h"
#include "Shared/Data/SkillDataAsset.h"
#include "Shared/GAS/AbilityTask/GGwaPlayMontageAndWaitForEvent.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTarget_Self.h"



UGA_Skill1::UGA_Skill1() {
	AbilityInputID = EAbilityInputID::Skill1;
	// FAbilityTriggerData TriggerData;
	// TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Skill.TargetLocation");
	// TriggerData.TriggerSource =	EGameplayAbilityTriggerSource::GameplayEvent;
	// AbilityTriggers.Add(TriggerData);
}

void UGA_Skill1::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {
	Super::OnAvatarSet(ActorInfo, Spec);
	// CooldownGameplayEffectClass = SkillDataAsset->CoolTimeGEClass;
}

void UGA_Skill1::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !CommitAbility(Handle, ActorInfo, ActivationInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	TWeakObjectPtr<AActor> AvatarActor = ActorInfo ? ActorInfo->AvatarActor : nullptr;
	PreProcessSkillStart(ActorInfo);
	if(TriggerEventData && TriggerEventData->TargetData.Num() > 0) {
		UE_LOG(LogTemp, Warning, TEXT("GA Server Handle Event Activate"));
		FVector Dir = TriggerEventData->TargetData.Get(0)->GetHitResult()->ImpactPoint - AvatarActor->GetActorLocation();
		AvatarActor->SetActorRotation(Dir.Rotation());
	}
	UGGwaPlayMontageAndWaitForEvent* Task = UGGwaPlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
							this,
							NAME_None,
							SkillDataAsset->CastMontage,
							FGameplayTagContainer(),
							1.0f,
							NAME_None,
							false
						);
	if (Task) {
		// GAS DOC의 기능을 이용하자 시간 너무 허비됨.
		Task->OnCompleted.AddDynamic(this, &UGA_Skill1::OnMontageCompleted);
		Task->OnBlendOut.AddDynamic(this, &UGA_Skill1::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);
		Task->OnCancelled.AddDynamic(this, &UGA_Skill1::OnMontageInterrupted);
		Task->EventReceived.AddDynamic(this, &UGA_Skill1::OnMontageEventReceived);
		Task->ReadyForActivation();
	}

	
	if (GetAvatarActorFromActorInfo()->HasAuthority()) {
		SkillContext = BuildSkillContext(ActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		SkillContext.DetectedActors = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);
		
		FGameplayEffectContextHandle Context = SkillContext.SourceASC->MakeEffectContext();
		Context.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);
		FGameplayEffectSpecHandle Spec = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, Context);
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(SkillAssetTypeTag), SkillContext.SkillData->SkillID);
		// *ULocalDataBaseLoader + RPC로 Data 전송이 대체 됨.
		if (Spec.IsValid()) {
			FGameplayEffectContextHandle CoolTimeContext = SkillContext.SourceASC->MakeEffectContext();
			CoolTimeContext.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);
			FGameplayEffectSpecHandle CoolTimeSpec = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->CoolTimeGEClass,1.f,CoolTimeContext);
			CoolTimeSpec.Data->SetSetByCallerMagnitude(SkillContext.SkillData->CooldownTag, SkillDataAsset->CoolTime);
			
			if (SkillContext.SkillData->TargetStrategyClass == USkillTarget_Self::StaticClass()) {
				SkillContext.SourceASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				SkillContext.SourceASC->ApplyGameplayEffectSpecToSelf(*CoolTimeSpec.Data.Get());
			}
			else if (SkillContext.DetectedActors.Num() > 0){
				for (auto* target : SkillContext.DetectedActors){
					if (UGGwaAbilitySystemComponent* ASC = GetTargetASC(target)) {
						// play monatge UAnimMontage* CastMontage;
						
						SkillContext.SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), ASC);
						SkillContext.SourceASC->ApplyGameplayEffectSpecToSelf(*CoolTimeSpec.Data.Get());
					}
				}
			}
		}
	}
	// else { // 클라이언트 기준으로만 CUE 실행
	
	const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec();
	if (CurrentSpec && CurrentSpec->IsActive())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}


void UGA_Skill1::OnMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData) {
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()){
		if (ASC->HasMatchingGameplayTag(SkillDataAsset->UniqueTag)) {
			// ASC->RemoveLooseGameplayTag(SkillDataAsset->UniqueTag);
			UE_LOG(LogTemp, Warning, TEXT("Montage Interrupted"));
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}
	}
}

void UGA_Skill1::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData) {
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()){
		if (ASC->HasMatchingGameplayTag(SkillDataAsset->UniqueTag)) {
			// ASC->RemoveLooseGameplayTag(SkillDataAsset->UniqueTag);
			UE_LOG(LogTemp, Warning, TEXT("Montage Completed"));
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
}

void UGA_Skill1::OnMontageEventReceived(FGameplayTag EventTag, FGameplayEventData EventData) {
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()){
		if (ASC->HasMatchingGameplayTag(SkillDataAsset->UniqueTag)) {
			// ASC->RemoveLooseGameplayTag(SkillDataAsset->UniqueTag);
			UE_LOG(LogTemp, Warning, TEXT("Montage EventReceived"));
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
}
