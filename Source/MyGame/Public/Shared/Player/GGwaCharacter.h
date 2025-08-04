
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "EnhancedInputComponent.h"
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
	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<FVector> CurrentPath;
	UPROPERTY(VisibleAnywhere, Replicated)
	bool bIsFollowingPath = false;
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 CurrentPathIndex = 1;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<UInputAction*> SkillActions;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> SkillAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	TSubclassOf<UGameplayAbility> MoveAbility;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	UFUNCTION(Client, reliable)
	void SetMoveData(const TArray<FVector>& Path, int32 PathIndex, bool bIsFollowing);

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void CustomKeySet(UInputAction* Action, FKey CustomKey);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPlayerReactionComponent* GetReactionComponent() const;
	UPROPERTY()
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;

protected:
	void InitASC();
private:
	void PlayerMove();
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	void OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index);
	TObjectPtr<UPlayerReactionComponent> ReactionComponent;
	TObjectPtr<USkillCastingService> SkillCastingService;
	
	FTimerHandle UIInitializationTimer;
};


