
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "UPlayerStateComponent.generated.h"

class AGGwaPlayerController;
class AGGwaCharacter;
class UGGwaAbilitySystemComponent;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpecificStateChanged, const FGameplayTag&, StateTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UPlayerStateComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UPlayerStateComponent();
	
	void InitComponent(UGGwaAbilitySystemComponent* AbilitySystemComponent);
	void RegisterForStateChange(const FGameplayTag& StateTag, const FOnSpecificStateChanged::FDelegate& Delegate);
	void UnregisterForStateChange(const FGameplayTag& StateTag, const FOnSpecificStateChanged::FDelegate& Delegate);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void UpdateStateTag(FGameplayTag ChangedTag, int32 NewCount);

	UFUNCTION()
	void OnRep_CurrentStateTag();
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	bool IsStateChangeable(const FGameplayTag NewStateTag) const;

	UFUNCTION()
	void HandleDeadState(const FGameplayTag& StateTag);

	UFUNCTION()
	void HandleStunnedState(const FGameplayTag& StateTag);

	UPROPERTY(ReplicatedUsing=OnRep_CurrentStateTag)
	FGameplayTag CurrentStateTag;

	UPROPERTY()
	TMap<FGameplayTag, FOnSpecificStateChanged> StateChangedEvents;
	
	UPROPERTY()
	TObjectPtr<AGGwaCharacter> Character;
	
	UPROPERTY()
	TObjectPtr<AGGwaPlayerController> PlayerController;
	
	UPROPERTY()
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
	
	UPROPERTY()
    TArray<FGameplayTag> MonitoredStateTags;
};