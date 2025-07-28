// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h" // Required for FOnAttributeChangeData
#include "UPlayerStateComponent.generated.h"

class AGGwaPlayerController;
class AGGwaCharacter;
class UGGwaAbilitySystemComponent;
struct FOnAttributeChangeData;

// New delegate for specific state changes. It passes the new state tag.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpecificStateChanged, const FGameplayTag&, StateTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UPlayerStateComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UPlayerStateComponent();
	
	/**
	 * Initializes the component with the Ability System Component and binds to necessary events.
	 * @param AbilitySystemComponent The ASC to use for this component.
	 */
	void InitComponent(UGGwaAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Register a callback for a specific state change.
	 * @param StateTag The state tag to listen for.
	 * @param Delegate The delegate to execute when the state is activated.
	 */
	void RegisterForStateChange(const FGameplayTag& StateTag, const FOnSpecificStateChanged::FDelegate& Delegate);

	/**
	 * Unregister a callback for a specific state change.
	 * @param StateTag The state tag to stop listening for.
	 * @param Delegate The delegate to remove.
	 */
	void UnregisterForStateChange(const FGameplayTag& StateTag, const FOnSpecificStateChanged::FDelegate& Delegate);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Called when a monitored tag's count changes. The entry point for state changes. */
	UFUNCTION()
	void UpdateStateTag(FGameplayTag ChangedTag, int32 NewCount);

	/** Called on clients when CurrentStateTag is replicated. */
	UFUNCTION()
	void OnRep_CurrentStateTag();
	
	/** Bound to the Health attribute change delegate on the ASC. */
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	/** Checks if the state can be changed to the new state. */
	bool IsStateChangeable(const FGameplayTag NewStateTag) const;

	/** Internal handler for the Dead state. */
	UFUNCTION()
	void HandleDeadState(const FGameplayTag& StateTag);

	/** Internal handler for the Stunned state. */
	UFUNCTION()
	void HandleStunnedState(const FGameplayTag& StateTag);

	/** The current primary state of the player. Replicated. */
	UPROPERTY(ReplicatedUsing=OnRep_CurrentStateTag)
	FGameplayTag CurrentStateTag;

	/** Map of state tags to the delegates that should be fired when they become active. */
	UPROPERTY()
	TMap<FGameplayTag, FOnSpecificStateChanged> StateChangedEvents;
	
	UPROPERTY()
	TObjectPtr<AGGwaCharacter> Character;
	
	UPROPERTY()
	TObjectPtr<AGGwaPlayerController> PlayerController;
	
	UPROPERTY()
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
	
	/** A list of state tags that this component will monitor for changes. */
	UPROPERTY()
    TArray<FGameplayTag> MonitoredStateTags;
};