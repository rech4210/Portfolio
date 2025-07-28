

#include "Shared/GAS/Skill/GA_Skill2.h"
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

void UGA_Skill2::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData*)
{
	FScopedPredictionWindow ScopedPredictionWindow(ActorInfo->AbilitySystemComponent.Get());

	if (!CanActivateAbility(Handle, ActorInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!ActorInfo || !CommitAbility(Handle, ActorInfo, ActivationInfo)){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// AvatarActor ��ȿ�� �˻�
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor)){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2) Ÿ���� Task
	if (SkillDataAsset->TargetStrategyClass->IsChildOf(USkillTarget_Self::StaticClass())){
		OnTargetDataReceived(FGameplayAbilityTargetDataHandle());
	}
	else
	{
		// ���콺 ��� ��ġ Ÿ����
		ASkillTargetActor_Mouse* TargetActor = NewObject<ASkillTargetActor_Mouse>(this);

		UAbilityTask_WaitTargetData* TargetTask = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
				this,
				FName("Skill2_Target"),
				EGameplayTargetingConfirmation::Instant,
				TargetActor
			);
		TargetTask->ValidData.AddDynamic(this, &UGA_Skill2::OnTargetDataReceived);
		TargetTask->Cancelled.AddDynamic(this, &UGA_Skill2::OnTargetDataCancelled);
		TargetTask->ReadyForActivation();
	}
}

void UGA_Skill2::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	// ��� �ÿ� Ability ����
	// GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(UEnumTagMatchHelper::GetTagFromEnum(ECueType::DirectionPreview));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UGA_Skill2::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data){
	
	if (!CurrentActorInfo){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor)){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//Cue �ð� ȿ�� ����
	if (Data.Num() > 0 && Data.Get(0)){
		const FHitResult* Hit = Data.Get(0)->GetHitResult();
		if (Hit){
			HitPoint = Hit->ImpactPoint;
			FVector Dir = Hit->ImpactPoint - AvatarActor->GetActorLocation();
			AvatarActor->SetActorRotation(Dir.Rotation());
			auto ASC = Cast<UGGwaAbilitySystemComponent>(GetActorInfo().AbilitySystemComponent);
			if (ASC && SkillDataAsset->GE_CueClass) {
				auto Context = ASC->MakeEffectContext();
				Context.AddInstigator(AvatarActor, AvatarActor);
				auto Spec = ASC->MakeOutgoingSpec(SkillDataAsset->GE_CueClass, 1.f, Context);
				Spec.Data->SetSetByCallerMagnitude(UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::CueDuration), SkillDataAsset->CueDuration);
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	// 4) ��Ÿ�� Task
	UGGwaPlayMontageAndWaitForEvent* MontageTask =
		UGGwaPlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
			this,
			NAME_None,
			SkillDataAsset->CastMontage,
			FGameplayTagContainer(),
			1.0f,
			NAME_None,
			false
		);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Skill2::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Skill2::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Skill2::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Skill2::OnMontageInterrupted);

	MontageTask->ReadyForActivation();
}

void UGA_Skill2::OnMontageInterrupted(FGameplayTag /*EventTag*/, FGameplayEventData /*EventData*/)
{
	GetActorInfo().AbilitySystemComponent->RemoveGameplayCue(UEnumTagMatchHelper::GetTagFromEnum(ECueType::DirectionPreview));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}


void UGA_Skill2::OnMontageCompleted(FGameplayTag ,FGameplayEventData){
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		SkillContext = BuildSkillContext(CurrentActorInfo);
		SkillContext.SkillData = SkillDataAsset;
		SkillContext.HitLocation =  HitPoint;
		SkillContext.DetectedActors = NewObject<USkillTargetBase>(this, SkillDataAsset->TargetStrategyClass)->DetectTargets(SkillContext);

		
		auto* ASC = SkillContext.SourceASC.Get();
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);

		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SkillDataAsset->GEClass, 1.f, Context);

		FGameplayEffectContextHandle CoolContext = ASC->MakeEffectContext();
		CoolContext.AddInstigator(SkillContext.SourceActor, SkillContext.SourceActor);

		FGameplayEffectSpecHandle CoolSpec = ASC->MakeOutgoingSpec(SkillDataAsset->GE_CoolTimeClass, 1.f, CoolContext);
		CoolSpec.Data->SetSetByCallerMagnitude(
			UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::Cooldown),
			SkillDataAsset->CoolTime
		);
		CoolSpec.Data->SetSetByCallerMagnitude(
			SkillAssetTypeTag,
			SkillDataAsset->SkillID
		);

		if (SkillDataAsset->TargetStrategyClass == USkillTarget_Self::StaticClass()){
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			ASC->ApplyGameplayEffectSpecToSelf(*CoolSpec.Data.Get());
		}
		else{
			for (AActor* Target : SkillContext.DetectedActors){
				if (UAbilitySystemComponent* TargetASC = GetTargetASC(Target) ){
					ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
				}
			}
			ASC->ApplyGameplayEffectSpecToSelf(*CoolSpec.Data.Get());
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true, false );
}
