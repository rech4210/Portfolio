// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillDomain.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "SkillDomainService.generated.h"

class USkillComponent;
class ISkillRepositoryInterface;
class APlayerState;
class USkillDataAsset;
struct FSkillSlotDTO;

// Application Events (발행: DomainService)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillOperationSucceeded, APlayerState* /* PlayerState */, const FString& /* Operation */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillOperationFailed, APlayerState* /* PlayerState */, const FString& /* Reason */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillLoadCompleted, APlayerState* /* PlayerState */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillSaveCompleted, APlayerState* /* PlayerState */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillRegistered, APlayerState* /* PlayerState */, const FSkillSlotDTO& /* SkillSlot */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillUnregistered, APlayerState* /* PlayerState */, const FGuid& /* SlotId */);

/**
 * Domain Service for Skill operations
 * Handles business logic orchestration and coordinates between Aggregates and Repositories
 */
UCLASS(BlueprintType)
class SKILLMODULE_API USkillDomainService : public UObject
{
	GENERATED_BODY()

public:
	USkillDomainService();

	/**
	 * Initialize the domain service with repository dependency
	 * @param Repository The repository interface to use for persistence
	 */
	void Initialize(TScriptInterface<ISkillRepositoryInterface> Repository);

	/**
	 * Domain Service: Register skill to player's skill slots with full business logic
	 * @param PlayerState Target player
	 * @param SkillData Skill data to register
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<bool> RegisterSkillToPlayer(APlayerState* PlayerState, USkillDataAsset* SkillData);

	/**
	 * Domain Service: Unregister skill from player's skill slots with full business logic
	 * @param PlayerState Target player
	 * @param SlotId Slot ID to unregister
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<bool> UnregisterSkillFromPlayer(APlayerState* PlayerState, const FGuid& SlotId);

	/**
	 * Domain Service: Swap skills between two slots with full business logic
	 * @param PlayerState Target player
	 * @param SlotIdA First slot ID
	 * @param SlotIdB Second slot ID
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<bool> SwapSkillSlots(APlayerState* PlayerState, const FGuid& SlotIdA, const FGuid& SlotIdB);

	/**
	 * Domain Service: Update skill cooldown state
	 * @param PlayerState Target player
	 * @param SlotId Slot ID
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that completes when operation finishes
	 */
	UE::Tasks::TTask<bool> UpdateSkillCooldown(APlayerState* PlayerState, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown);

	/**
	 * Domain Service: Load player's skills from persistence
	 * @param PlayerState Target player
	 * @return Task that completes when loading finishes
	 */
	UE::Tasks::TTask<bool> LoadSkills(APlayerState* PlayerState);

	/**
	 * Domain Service: Save player's current skill state
	 * @param PlayerState Target player
	 * @return Task that completes when save finishes
	 */
	UE::Tasks::TTask<bool> SaveSkills(APlayerState* PlayerState);

	// Application Events
	FOnSkillOperationSucceeded OnSkillOperationSucceeded;
	FOnSkillOperationFailed OnSkillOperationFailed;
	FOnSkillLoadCompleted OnSkillLoadCompleted;
	FOnSkillSaveCompleted OnSkillSaveCompleted;
	FOnSkillRegistered OnSkillRegistered;
	FOnSkillUnregistered OnSkillUnregistered;

private:
	// Repository dependency (injected)
	UPROPERTY()
	TScriptInterface<ISkillRepositoryInterface> SkillRepository;

	/**
	 * Subscribe to domain events from SkillComponent
	 */
	void SubscribeToDomainEvents(USkillComponent* SkillComponent);

	/**
	 * Unsubscribe from domain events
	 */
	void UnsubscribeFromDomainEvents(USkillComponent* SkillComponent);

	// Domain event handlers
	UFUNCTION()
	void OnDomainSkillRegistered(USkillDataAsset* SkillData);

	UFUNCTION()
	void OnDomainSkillUnregistered(const FGuid& SlotId);

	UFUNCTION()
	void OnDomainSkillsChanged();
};
