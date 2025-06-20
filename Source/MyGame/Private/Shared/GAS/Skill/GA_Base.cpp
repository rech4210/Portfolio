// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Skill/GA_Base.h"
#include "AbilitySystemInterface.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Shared/AI/BossCharacter.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"
#include "Shared/AI/EnemyAttributeSet.h"
#include "Shared/Data/EGasDataType.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"

const FGameplayTag UGA_Base::SkillAssetTypeTag = UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::SkillID);

void UGA_Base::PreProcessSkillStart(const FGameplayAbilityActorInfo* ActorInfo) {
	AGGwaCharacter * AvatarActor = Cast<AGGwaCharacter>(ActorInfo->AvatarActor.Get());
	if (IsLocallyControlled()) {
		UE_LOG(LogTemp, Warning, TEXT("✅ Client PreProcess Activated"));
		UGGwaAbilitySystemComponent* ASC = Cast<UGGwaAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
		
		if (AGGwaPlayerController * PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController.Get())) {
			// FHitResult Hit;
			// if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
			// 	CacheHitLocation = Hit.ImpactPoint;
			// 	FVector Dir = Hit.ImpactPoint - AvatarActor->GetActorLocation();
			// 	Dir.Z = 0;
			// 	
			// 	AvatarActor->SetActorRotation(Dir.Rotation());
			// 	FGameplayEventData RotationEventData;
			// 	RotationEventData.Instigator = AvatarActor;
			// 	RotationEventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
			// 	ActorInfo->AbilitySystemComponent->HandleGameplayEvent(RotationEventData.EventTag ,&RotationEventData);
			// }
		}
	}
	// else {
	// 	if(TriggeredEventData && TriggeredEventData->TargetData.Num() > 0) {
	// 		FVector Dir = TriggeredEventData->TargetData.Get(0)->GetHitResult()->ImpactPoint - AvatarActor->GetActorLocation();
	// 		AvatarActor->SetActorRotation(Dir.Rotation());
	// 	}
	// }
}
UAbilitySystemComponent* UGA_Base::GetTargetASC(AActor* Actor) const {
	//캐릭터가 아닌 경우?
	if (AGGwaCharacter* Character = Cast<AGGwaCharacter>(Actor)){
		if (AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(Character->GetPlayerState())) {
			return PS->GetAbilitySystemComponent();
		}
		return nullptr;
	}
	if (ABossCharacter* Boss = Cast<ABossCharacter>(Actor)) {
		if (UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent()) {
			return ASC;
		}
		return nullptr;
	}
	return nullptr;
}

FSkillContext UGA_Base::BuildSkillContext(const FGameplayAbilityActorInfo* ActorInfo) {
	FSkillContext Context;
	Context.SourceASC = ActorInfo->AbilitySystemComponent.Get();
	Context.SourceActor = ActorInfo->AvatarActor.Get();
	Context.TargetLocation = CacheHitLocation;
	return Context;
}
