// UPlayerStateComponent.cpp
#include "Shared/Player/Component/UPlayerStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"
#include "Shared/Player/EPlayerState.h"

UPlayerStateComponent::UPlayerStateComponent() {
    SetIsReplicatedByDefault(true);
    TagStunned = UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Stunned);
    TagKnockback = UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Knockback);
    TagDead = UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Dead);
}

void UPlayerStateComponent::PlayerDeath() {
    OnPlayerStateChanged.Broadcast(EPlayerState::Dead);
}

//
// void UPlayerStateComponent::Initialize(UGGwaAbilitySystemComponent* ASC) {
//     ASC->RegisterGameplayTagEvent(
//         TagStunned, EGameplayTagEventType::NewOrRemoved)
//         .AddUObject(this, &UPlayerStateComponent::HandleTagChanged);
//
//     ASC->RegisterGameplayTagEvent(
//         TagKnockback, EGameplayTagEventType::NewOrRemoved)
//         .AddUObject(this, &UPlayerStateComponent::HandleTagChanged);
//
//     ASC->RegisterGameplayTagEvent(
//         TagDead, EGameplayTagEventType::NewOrRemoved)
//         .AddUObject(this, &UPlayerStateComponent::HandleTagChanged);
// }
//
// void UPlayerStateComponent::HandleTagChanged(
//     const FGameplayTag Tag, int32 NewCount) {
//     UEnumTagMatchHelper::Register<EPlayerState>(EPlayerState::Combat, TagDead);
//
//     EPlayerState Next = CurrentState;
//
//     if (Tag == TagKnockback && NewCount > 0)
//         Next = EPlayerState::Knockback;
//     else if (Tag == TagStunned && NewCount > 0)
//         Next = EPlayerState::Stunned;
//     else if (Tag == TagDead && NewCount > 0)
//         Next = EPlayerState::Dead;
//     else if (NewCount == 0)
//         Next = EPlayerState::Combat;
//
//     if (Next != CurrentState) {
//         CurrentState = Next;
//         OnRep_PlayerState();
//     }
// }
//
// void UPlayerStateComponent::OnRep_PlayerState() {
//     OnPlayerStateChanged.Broadcast(CurrentState);
// }
//
//
// void UPlayerStateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
//     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//     DOREPLIFETIME(UPlayerStateComponent, CurrentState);
// }