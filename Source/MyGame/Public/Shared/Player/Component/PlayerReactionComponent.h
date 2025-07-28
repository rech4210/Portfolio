
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerReactionComponent.generated.h"


class AGGwaPlayerState;
class AGGwaCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UPlayerReactionComponent : public UActorComponent {
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	UFUNCTION()
	void HandleGameplayCue(const FGameplayTag EventTag,EGameplayCueEvent::Type CueType, const FGameplayCueParameters& Params);
	UFUNCTION()
	void Init(UGGwaAbilitySystemComponent* ASC);
	UFUNCTION()
	void ExecuteDeadReaction(const FGameplayTag& StateTag);
	UFUNCTION()
	void ExecuteKnockback(const FGameplayTag& StateTag);

private:
	void ExecuteKnockback();
	UPROPERTY()
	TObjectPtr<AGGwaCharacter> Character;
	UPROPERTY()
	TObjectPtr<AGGwaPlayerState> PlayerState;
};
