// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GGwaCharacter.generated.h"

struct FGameplayEventData;
class UInputMappingContext;
class UInputAction;
class UGGwaAbilitySystemComponent;
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
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<FGameplayTag> AbilityTags;
	UPROPERTY(EditDefaultsOnly, category = "Abilities")
	TArray<UAnimMontage*> SkillMontages;

	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	UFUNCTION(Server, Reliable, WithValidation)
	void OnSkillTriggered(const FGameplayEventData& EventData, int32 Index);
	// UFUNCTION(BlueprintImplementableEvent,Category="input")
	// void OnMouseClickActionPressed(const FInputActionInstance& InputActionInstance);
	UFUNCTION(client, reliable)
	void SetMoveData(const TArray<FVector>& Path, int32 PathIndex, bool bIsFollowing);
protected:
	UPROPERTY()
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
	// UPROPERTY()
	// TObjectPtr<UGGwaAttributeSet> AttributeSet;
	void InitASC();
private:
	void ExecuteAbility(const FGameplayEventData& EventData, int32 Index);
	void OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index);
};


