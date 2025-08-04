#include "Shared/Player/Component/PlayerReactionComponent.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "GameSharedModule/Public/Enum/ECueType.h"
#include "GameSharedModule/Public/Enum/EPlayerState.h"
#include "Shared/Player/GGwaCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/Player/Component/UPlayerStateComponent.h"

void UPlayerReactionComponent::BeginPlay() {
	Super::BeginPlay();
	if(Character)
	{
		if(AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(Character->GetPlayerState()))
		{
			if(UPlayerStateComponent* StateComponent = PS->GetStateComponent())
			{
				FOnSpecificStateChanged::FDelegate DeadDelegate;
				DeadDelegate.BindDynamic(this, &UPlayerReactionComponent::ExecuteDeadReaction);
				StateComponent->RegisterForStateChange(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Dead), DeadDelegate);

				FOnSpecificStateChanged::FDelegate KnockbackDelegate;
				KnockbackDelegate.BindDynamic(this, &UPlayerReactionComponent::ExecuteKnockback);
				StateComponent->RegisterForStateChange(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Knockback), KnockbackDelegate);
			}
		}
	}
}

void UPlayerReactionComponent::HandleGameplayCue(const FGameplayTag EventTag , EGameplayCueEvent::Type CueType, const FGameplayCueParameters& Params) { 
	if (EventTag == UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Knockback)) {
	}
	if (EventTag == UEnumTagMatchHelper::GetTagFromEnum<ECueType>(ECueType::AreaAttackWarning)) {
		UGameplayStatics::SpawnEmitterAtLocation(
			this, /* 파티클 어셋 참조 */ nullptr,
			GetOwner()->GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(
			this, /* 사운드 어셋 참조 */ nullptr,
			GetOwner()->GetActorLocation());
	}
}

void UPlayerReactionComponent::Init(UGGwaAbilitySystemComponent* ASC) {
}

void UPlayerReactionComponent::ExecuteKnockback(const FGameplayTag& StateTag) {
	if (GetOwner()->HasAuthority()) {
		return;
	}
	
	UGameplayStatics::SpawnEmitterAtLocation(
		this, /* 파티클 어셋 참조 */ nullptr,
		GetOwner()->GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(
		this, /* 사운드 참조 */ nullptr,
		GetOwner()->GetActorLocation());
}

void UPlayerReactionComponent::ExecuteDeadReaction(const FGameplayTag& StateTag) {
	AGGwaCharacter* OwnerCharacter = Cast<AGGwaCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->ASC;
	if (!ASC)
	{
		return;
	}
	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	
	if (GetOwner()->HasAuthority()) {
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		// TODO: Notify GameMode
	}
	else {
		// TODO: Play death montage from data
		OwnerCharacter->GetMesh()->SetSimulatePhysics(true);
	}
}
