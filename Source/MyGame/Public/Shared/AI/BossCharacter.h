// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "EnemySystemCore/FBossDataStruct.h"
#include "GameFramework/Character.h"
#include "BossCharacter.generated.h"

class UGameplayAbility;
class UBossAttributeObserverComponent;
class UBossSkillComponent;
class UEnemyAbilitySystemComponent;
class UEnemyAttributeSet;

UCLASS()
class MYGAME_API ABossCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABossCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
	
	TArray<AActor*> DetectTarget(float Radius) const;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void InitASC();
	void UpdateDataFromBoss(FBossDataStruct & Data);
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	UPROPERTY(ReplicatedUsing = OnRep_BossData)
	FBossDataStruct CachedBossData;

	UFUNCTION()
	void OnRep_BossData();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
private:
	TObjectPtr<UEnemyAbilitySystemComponent> E_ASC;
	TObjectPtr<UBossSkillComponent> SkillComponent;
	TObjectPtr<UEnemyAttributeSet> E_AttributeSet;
	
	TObjectPtr<UBossAttributeObserverComponent> AttributeObserverComponent;
};
