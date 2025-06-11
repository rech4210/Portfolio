// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "BossAttributeObserverComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UBossAttributeObserverComponent : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBossAttributeObserverComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;
	void BindBossDataDelegate();
};
