// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GGwaCharacter.generated.h"

class USkillCastingService;
class UInventoryComponent;
class USkillComponent;
class UPlayerReactionComponent;
struct FGameplayEventData;
class UInputMappingContext;
class UInputAction;
class UGGwaAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class MYGAME_API AGGwaCharacter : public ACharacter, public IAbilitySystemInterface
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

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void CustomKeySet(UInputAction* Action, FKey CustomKey);

	UFUNCTION(client, reliable)
	void SetMoveData(const TArray<FVector>& Path, int32 PathIndex, bool bIsFollowing);

	UPlayerReactionComponent* GetReactionComponent() const;
	UPROPERTY()
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
protected:
	void InitASC();
private:
	virtual void PostInitializeComponents() override;
	void OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index);
	TObjectPtr<UPlayerReactionComponent> ReactionComponent;
	TObjectPtr<USkillCastingService> SkillCastingService;
};


