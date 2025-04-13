// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Skill/GA_Skill2.h"

#include "AbilitySystemInterface.h"
#include "Shared/GAS/SkillTargetPolicy/SkillTarget_TargetActor.h"


void UGA_Skill2::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	const UAbilityTask_WaitTargetData* TargetDataTask = GetTargetDataTask();
	/*의문 : 여기서 어떤 기준으로 OnTargetDataReceived, OnTargetDataCancelled 호출하지?*/
	// OnTargetDataReceived : HitData->HitResult.ImpactPoint;
}


/*아래 내용들은 Interface에 정의하여, AbilityTask가 필요한 기술일 경우 필수 구현하도록 순수가상함수 설정.*/
void UGA_Skill2::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data) {
	if (const FGameplayAbilityTargetData_SingleTargetHit* HitData = static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(Data.Get(0))){
		/* 여기서 부모를 호출하는 이유는, Task가 성공 후에 해당 방향을 바라보도록 부모 로직을 처리하기 위해서임.*/
		PreProcessSkillStart(CurrentActorInfo);
		SkillContext = BuildSkillContext(CurrentActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		
		FVector HitLocation = HitData->HitResult.ImpactPoint;
		SkillContext.TargetLocation = HitLocation;
		SkillContext.TargetActor = HitData->GetHitResult()->GetActor();
		// 범위 공격일 경우 사용
		SkillContext.DetectedActors = NewObject<USkillTarget_TargetActor>
									(SkillDataAsset->TargetStrategyClass)
									->DetectTargets(SkillContext);
		FGameplayEffectSpecHandle SpecHandle = SkillContext.SourceASC->MakeOutgoingSpec(SkillDataAsset->GEClass,
											1.f, SkillContext.SourceASC->MakeEffectContext());
		if (SkillContext.TargetActor->Implements<UAbilitySystemInterface>()) {
			UAbilitySystemComponent* TargetASC = Cast<IAbilitySystemInterface>(SkillContext.TargetActor)->GetAbilitySystemComponent();
			if (TargetASC && SpecHandle.IsValid()) {
				SkillContext.SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true,false);
}

void UGA_Skill2::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data) {
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true,true);
}

UAbilityTask_WaitTargetData* UGA_Skill2::GetTargetDataTask() {
	UAbilityTask_WaitTargetData* WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
		this,
		NAME_None,
		EGameplayTargetingConfirmation::UserConfirmed,
		AbilityTargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this, &UGA_Skill2::OnTargetDataReceived);
	WaitTargetData->Cancelled.AddDynamic(this, &UGA_Skill2::OnTargetDataCancelled);
	WaitTargetData->ReadyForActivation();
	return WaitTargetData;
}