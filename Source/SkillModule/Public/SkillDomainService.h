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
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillDomainRegistered, const FGuid& /* PlayerGuid */, const FSkillSlotDTO& /* SkillSlot */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillDomainUnregistered, const FGuid& /* PlayerGuid */, const FGuid& /* SlotId */);

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

	// ========================================================================
	// MODERN SKILL SYSTEM METHODS (RECOMMENDED)
	// ========================================================================

	/**
	 * Load all player's skills and skill slots
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to load skills for
	 */
	void LoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	/**
	 * Save player's skills
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to save skills for
	 */
	void SavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	/**
	 * Update player's skill slot
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to update skills for
	 * @param SlotIndex Slot index to update
	 * @param SkillData New skill data for the slot (null to unregister)
	 */
	void UpdatePlayerSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData);

	/**
	 * Update skill cooldown
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to update cooldown for
	 * @param SlotIndex Slot index to update
	 * @param LastUsedTime When the skill was last used
	 */
	void UpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime);

	/**
	 * Clear all skill slots for a player
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to clear slots for
	 */
	void ClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	// Domain Events
	FOnSkillOperationSucceeded OnSkillOperationSucceeded;
	FOnSkillOperationFailed OnSkillOperationFailed;
	FOnSkillLoadCompleted OnSkillLoadCompleted;
	FOnSkillSaveCompleted OnSkillSaveCompleted;
	FOnSkillDomainRegistered OnSkillDomainRegistered;
	FOnSkillDomainUnregistered OnSkillDomainUnregistered;

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
