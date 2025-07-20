// UPlayerStateComponent.cpp
#include "Shared/Player/Component/UPlayerStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "GameSharedModule/Public/Enum/EPlayerState.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "GameFramework/Pawn.h"

UPlayerStateComponent::UPlayerStateComponent() {
    SetIsReplicatedByDefault(true);
}

void UPlayerStateComponent::RegisterForStateChange(const FGameplayTag& StateTag, const FOnSpecificStateChanged::FDelegate& Delegate)
{
    StateChangedEvents.FindOrAdd(StateTag).Add(Delegate);
}

void UPlayerStateComponent::UnregisterForStateChange(const FGameplayTag& StateTag, const FOnSpecificStateChanged::FDelegate& Delegate)
{
    if(FOnSpecificStateChanged* FoundDelegate = StateChangedEvents.Find(StateTag))
    {
        FoundDelegate->Remove(Delegate);
    }
}

void UPlayerStateComponent::UpdateStateTag(FGameplayTag ChangedTag, int32 NewCount){
    if (Character && Character->HasAuthority()) {
        // Tag added
        if (NewCount > 0)
        {
            if (!IsStateChangeable(ChangedTag)) {
                return;
            }
            CurrentStateTag = ChangedTag;
            
            // Broadcast to specific listeners
            if(FOnSpecificStateChanged* FoundDelegate = StateChangedEvents.Find(ChangedTag))
            {
                FoundDelegate->Broadcast(ChangedTag);
            }
            // Also broadcast to client via OnRep
            OnRep_CurrentStateTag();
        }
        // Tag removed
        else
        {
            if (CurrentStateTag == ChangedTag)
            {
                CurrentStateTag = UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Combat);
                if(FOnSpecificStateChanged* FoundDelegate = StateChangedEvents.Find(CurrentStateTag))
                {
                    FoundDelegate->Broadcast(CurrentStateTag);
                }
                OnRep_CurrentStateTag();
            }
        }
    }
}

void UPlayerStateComponent::OnRep_CurrentStateTag(){
    if (!PlayerController && Character)
    {
        if(APawn* Pawn = Cast<APawn>(Character))
        {
            PlayerController = Cast<AGGwaPlayerController>(Pawn->GetController());
        }
    }
    if (PlayerController)
    {
        // This function seems to be for client-side notification.
        // The original implementation had this. Let's assume it's correct.
        PlayerController->NotifyStateChanged();
    }
    
    // Broadcast to local listeners on the client as well
    if(FOnSpecificStateChanged* FoundDelegate = StateChangedEvents.Find(CurrentStateTag))
    {
        FoundDelegate->Broadcast(CurrentStateTag);
    }
}

void UPlayerStateComponent::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if(Data.NewValue <= 0.f && Data.OldValue > 0.f)
    {
        if(ASC)
        {
            ASC->AddReplicatedLooseGameplayTag(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Dead));
        }
    }
}


void UPlayerStateComponent::HandleDeadState(const FGameplayTag& StateTag)
{
    if (ASC && ASC->HasMatchingGameplayTag(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Dead))) {
        ASC->CancelAllAbilities();
    }
    // Death logic here
}

void UPlayerStateComponent::HandleStunnedState(const FGameplayTag& StateTag)
{
    if (ASC && ASC->HasMatchingGameplayTag(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Stunned))) {
        ASC->CancelAllAbilities();
    }
}

void UPlayerStateComponent::InitComponent(UGGwaAbilitySystemComponent* AbilitySystemComponent) {
    Character = Cast<AGGwaCharacter>(AbilitySystemComponent->GetAvatarActor());
    if (!Character) return;

    if(APawn* Pawn = Cast<APawn>(Character))
    {
        PlayerController = Cast<AGGwaPlayerController>(Pawn->GetController());
    }
    
    ASC = AbilitySystemComponent;
    if (!ASC) {
        UE_LOG(LogTemp, Error, TEXT("UPlayerStateComponent::InitComponent() Failed, ASC is Nullptr"));
        return;
    }
    
    MonitoredStateTags.Empty();
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Idle));
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Combat));
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Stunned));
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Knockback));
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Dead));
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Hit));
    MonitoredStateTags.Add(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Guard));
    
    for (const FGameplayTag& Tag : MonitoredStateTags) {
        if(Tag.IsValid())
        {
            ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UPlayerStateComponent::UpdateStateTag);
        }
    }

    // Bind to health changes for death detection
    const UGGwaAttributeSet* AttributeSet = ASC->GetSet<UGGwaAttributeSet>();
    if(AttributeSet)
    {
        ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UPlayerStateComponent::OnHealthChanged);
    }
    
    // Register internal handlers
    FOnSpecificStateChanged::FDelegate DeadDelegate;
    DeadDelegate.BindDynamic(this, &UPlayerStateComponent::HandleDeadState);
    RegisterForStateChange(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Dead), DeadDelegate);

    FOnSpecificStateChanged::FDelegate StunnedDelegate;
    StunnedDelegate.BindDynamic(this, &UPlayerStateComponent::HandleStunnedState);
    RegisterForStateChange(UEnumTagMatchHelper::GetTagFromEnum<EPlayerState>(EPlayerState::Stunned), StunnedDelegate);
}

bool UPlayerStateComponent::IsStateChangeable(const FGameplayTag NewStateTag) const {
    if (CurrentStateTag == NewStateTag) {
        return false;
    }
    // TODO: Add logic here to prevent state changes (e.g., cannot go from Dead to Stunned)
    return true;
}

void UPlayerStateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPlayerStateComponent, CurrentStateTag);
}

// TODO
// Notify 호출 최적화 (UI 기준으로, 모든 UI들이 상태 변화시마다 호출되면 불필요하다고 판단
// Server 연산 기준으로, 현재 state 변경시, 제어되어야 할 함수 분기를 아직 설정하지 않음.
// 상태가 필요하다 -> On Rep, 즉각적 명령 -> RPC
// 상태가 필요하다 -> On Rep, 즉각적 명령 -> RPC