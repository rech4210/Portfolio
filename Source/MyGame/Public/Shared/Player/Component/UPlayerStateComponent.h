// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Shared/Player/EPlayerState.h"
#include "UPlayerStateComponent.generated.h"



class UGGwaAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, EPlayerState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UPlayerStateComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UPlayerStateComponent();

	// void Initialize(UGGwaAbilitySystemComponent* ASC);

	// UPROPERTY(ReplicatedUsing=OnRep_PlayerState)
	// EPlayerState CurrentState;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChanged OnPlayerStateChanged;

	void PlayerDeath();
	// virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:

private:
	// void HandleTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	FGameplayTag TagStunned;
	FGameplayTag TagKnockback;
	FGameplayTag TagDead;
};