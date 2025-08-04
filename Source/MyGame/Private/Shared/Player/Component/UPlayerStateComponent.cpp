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
        if (NewCount > 0)
        {
            if (!IsStateChangeable(ChangedTag)) {
                return;
            }
            CurrentStateTag = ChangedTag;
            
            if(FOnSpecificStateChanged* FoundDelegate = StateChangedEvents.Find(ChangedTag))
            {
                FoundDelegate->Broadcast(ChangedTag);
            }
            OnRep_CurrentStateTag();
        }
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
        PlayerController->NotifyStateChanged();
    }
    
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

    const UGGwaAttributeSet* AttributeSet = ASC->GetSet<UGGwaAttributeSet>();
    if(AttributeSet)
    {
        ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UPlayerStateComponent::OnHealthChanged);
    }
    
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
    return true;
}

void UPlayerStateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPlayerStateComponent, CurrentStateTag);
}
