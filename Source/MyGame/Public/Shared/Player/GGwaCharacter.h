// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GGwaCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
class UGameplayAbility;
// class UGGwaAttributeSet;

UCLASS()
class MYGAME_API AGGwaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGGwaCharacter();
	
	UPROPERTY(EditAnywhere)
	float AcceptanceRadius = 50.0f;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> CurrentPath;
	UPROPERTY()
	bool bIsFollowingPath = false;
	
	int32 CurrentPathIndex = 1;
	FTimerHandle MoveTickHandle;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<UInputAction*> SkillActions;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> SkillAbilities;

	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void OnSkillTriggered(const FInputActionInstance& Instance, int32 Index);
	// UFUNCTION(BlueprintImplementableEvent,Category="input")
	// void OnMouseClickActionPressed(const FInputActionInstance& InputActionInstance);
	void SetMoveData(TArray<FVector> CurrentPath, int32 CurrentPathIndex, bool bIsFollowingPath);
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	// UPROPERTY()
	// TObjectPtr<UGGwaAttributeSet> AttributeSet;
	void InitASC();
};


