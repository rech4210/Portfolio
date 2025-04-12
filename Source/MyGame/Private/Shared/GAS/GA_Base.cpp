// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/GA_Base.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"

void UGA_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	AGGwaCharacter * AvatarActor = Cast<AGGwaCharacter>(ActorInfo->AvatarActor.Get());
	UE_LOG(LogTemp, Warning, TEXT("✅ UGA_Base Activated"));
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	
	if (AGGwaPlayerController * PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController.Get())) {
		FHitResult Hit;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
			CacheHitLocation = Hit.ImpactPoint;
			FVector Dir = Hit.ImpactPoint - AvatarActor->GetActorLocation();
			Dir.Z = 0;
			
			AvatarActor->SetActorRotation(Dir.Rotation());
		}
	}
	K2_ActivateAbility();
}

FSkillContext UGA_Base::BuildSkillContext(const FGameplayAbilityActorInfo* ActorInfo) {
	FSkillContext Context;
	Context.SourceASC = ActorInfo->AbilitySystemComponent.Get();
	Context.SourceActor = ActorInfo->AvatarActor.Get();
	Context.TargetLocation = CacheHitLocation;
	return Context;
}
