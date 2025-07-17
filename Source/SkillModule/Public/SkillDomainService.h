// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillDomain.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "Interface/PlayerIdentityInterface.h"
#include "SkillDomainService.generated.h"

class USkillComponent;
class ISkillRepositoryInterface;
class USkillDataAsset;
struct FSkillSlotDTO;

// Domain Events (발행: DomainService)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillOperationSucceeded, const FGuid& /* PlayerGuid */, const FString& /* Operation */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillOperationFailed, const FGuid& /* PlayerGuid */, const FString& /* Reason */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillLoadCompleted, const FGuid& /* PlayerGuid */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillSaveCompleted, const FGuid& /* PlayerGuid */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillRegistered, const FGuid& /* PlayerGuid */, const FSkillSlotDTO& /* SkillSlot */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillUnregistered, const FGuid& /* PlayerGuid */, const FGuid& /* SlotId */);

/**
 * Domain Service for Skill operations
 * Handles business logic orchestration with atomic transactions
 * Uses void return types with domain events for UI updates
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
	 * Uses atomic transaction - triggers domain events for success/failure
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SkillData Skill data to register
	 */
	void RegisterSkillToPlayer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillDataAsset* SkillData);

	/**
	 * Domain Service: Unregister skill from player's skill slots with full business logic
	 * Uses atomic transaction - triggers domain events for success/failure
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SlotId Slot ID to unregister
	 */
	void UnregisterSkillFromPlayer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotId);

	/**
	 * Domain Service: Swap skills between two slots with full business logic
	 * Uses atomic transaction - triggers domain events for success/failure
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SlotIdA First slot ID
	 * @param SlotIdB Second slot ID
	 */
	void SwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotIdA, const FGuid& SlotIdB);

	/**
	 * Domain Service: Update skill cooldown state
	 * Uses atomic transaction - triggers domain events for success/failure
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SlotId Slot ID
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 */
	void UpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown);

	/**
	 * Domain Service: Load player's skills from persistence
	 * Triggers domain events for success/failure
	 * @param PlayerState Target player state containing SkillComponent
	 */
	void LoadSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);

	/**
	 * Domain Service: Save player's current skill state
	 * Uses atomic transaction - triggers domain events for success/failure
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SkillData The skill domain data to save
	 */
	void SaveSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FSkillDomain& SkillData);

	// Domain Events
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
	 * Execute repository operation with error handling and domain event emission
	 * @param RepositoryTask The repository task to execute
	 * @param PlayerId The player ID for event emission
	 * @param OperationName The operation name for logging
	 */
	template<typename T>
	void ExecuteWithEvents(UE::Tasks::TTask<T> RepositoryTask, const FGuid& PlayerGuid, const FString& OperationName);
};
