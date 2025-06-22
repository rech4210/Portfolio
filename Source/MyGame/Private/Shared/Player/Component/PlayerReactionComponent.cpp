// UPlayerReactionComponent.cpp
#include "Shared/Player/Component/PlayerReactionComponent.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"
#include "Shared/Player/EPlayerState.h"
#include "Shared/Player/GGwaCharacter.h"

void UPlayerReactionComponent::Initialize(
	UGGwaAbilitySystemComponent* ASC) {
	// ASC->OnGameplayCueReceived.AddDynamic(
	// 	this, &UPlayerReactionComponent::HandleGameplayCue);
}

void UPlayerReactionComponent::HandleGameplayCue(const FGameplayTag EventTag/*change Cue Enum*/ , EGameplayCueEvent::Type CueType, const FGameplayCueParameters& Params) { 
	if (EventTag == UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Knockback)) {
		// 실제 연산은 GE Execution이 수행하며, 이후 로직은 클라이언트 시각용
	}
}

void UPlayerReactionComponent::ExecuteKnockback(
	const FVector& Velocity) {
	// 서버 권한에서만 물리 이동
	if (GetOwner()->HasAuthority()) {
		return;
	}
	// 클라 시각/청각 효과
	// PlayCameraShake();

	
	UGameplayStatics::SpawnEmitterAtLocation(
		this, /* 파티클 어셋 참조 */ nullptr,
		GetOwner()->GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(
		this, /* 사운드 참조 */ nullptr,
		GetOwner()->GetActorLocation());
}

void UPlayerReactionComponent::ExecuteDeadReaction() {
        if (AGGwaCharacter* Character = Cast<AGGwaCharacter>(GetOwner()))
        {
                if (Character->HasAuthority())
                {
                        Character->DetachFromControllerPendingDestroy();
                }
                else
                {
                        Character->DisableInput(nullptr);
                }
        }
}
