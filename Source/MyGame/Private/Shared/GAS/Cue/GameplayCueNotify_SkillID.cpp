// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GAS/Cue/GameplayCueNotify_SkillID.h"

#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"

bool UGameplayCueNotify_SkillID::OnExecute_Implementation(AActor* MyTarget,const FGameplayCueParameters& Parameters) const {
	int32 SkillId = FMath::RoundToInt32( Parameters.RawMagnitude);
	// Client Logic Only
	if (!MyTarget->GetOwner()->HasAuthority()) {
		if (APawn* Pawn = Cast<APawn>(MyTarget)){
			if (AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(Pawn->GetController())) {
				return true;
			}
		}
	}
	return false;
}
