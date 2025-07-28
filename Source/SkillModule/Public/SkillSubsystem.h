#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "Interface/PlayerIdentityInterface.h"
#include "SkillSubsystem.generated.h"

class USkillConfigRepository;
class USkillStateRepository;
class USkillRepository;
class USkillDomainService;
class USkillComponent;
class USkillDataAsset;
struct FSkillDomain;

// Delegate for skill data loading completion callback
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillDataLoadCompleted, TScriptInterface<IPlayerIdentityInterface>, PlayerIdentity, USkillComponent*, SkillComponent);

/**
 * Skill Subsystem - Pure Repository Management
 * Responsibility: Only manages repository instances and provides DI for domain services
 * Does NOT contain business logic, validation, or event handling
 */
UCLASS()
class SKILLMODULE_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Get the current skill repository interface (for dependency injection)
	 * @return Current repository implementation
	 */
	TScriptInterface<ISkillRepositoryInterface> GetSkillRepository() const;

	/**
	 * Set skill repository implementation (Dependency Injection)
	 * @param Repository The repository implementation to use (MySQL/NoSQL/etc)
	 */
	void SetSkillRepository(TScriptInterface<ISkillRepositoryInterface> Repository);

	/**
	 * Create and configure a Domain Service instance
	 * @return Configured domain service with repository dependency injected
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	USkillDomainService* GetDomainService();

	// ============================================================================
	// MODERN SKILL SYSTEM USE CASES (RECOMMENDED)
	// ============================================================================

	/**
	 * Request player skill loading - loads all skill slots and master data for player
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to load skills for (UUID string format)
	 */
	void RequestLoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	/**
	 * Request player skill saving - saves all skill slots for player
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to save skills for (UUID string format)
	 */
	void RequestSavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	/**
	 * Request skill slot update
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to update skills for (UUID string format)
	 * @param SlotIndex Slot index to update
	 * @param SkillData New skill data for the slot (null to unregister)
	 */
	void RequestUpdateSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData);

	/**
	 * Request skill cooldown update
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to update cooldown for (UUID string format)
	 * @param SlotIndex Slot index to update
	 * @param LastUsedTime When the skill was last used
	 */
	void RequestUpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime);

	/**
	 * Request clearing all skill slots
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to clear slots for
	 */
	void RequestClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	/**
	 * Request skill swap
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to swap skills for
	 * @param SlotIndexA First slot index
	 * @param SlotIndexB Second slot index
	 */
	void RequestSwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndexA, int32 SlotIndexB);

	// ============================================================================
	// LEGACY USE CASE ORCHESTRATION - DEPRECATED
	// ============================================================================

	// /**
	//  * DEPRECATED: Request skill registration with full validation pipeline
	//  * Use 3-Layer Mapping Architecture: RequestUpdateSkillSlot3Layer() instead
	//  * Handles network authority, logging, and transaction boundaries
	//  * @param PlayerIdentity Target player identity containing SkillComponent
	//  * @param SkillData Skill data to register
	//  */
	// UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: RequestUpdateSkillSlot3Layer() instead")
	// void RequestRegisterSkill(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillDataAsset* SkillData);
	//
	// /**
	//  * DEPRECATED: Request skill unregistration with full validation pipeline
	//  * Use 3-Layer Mapping Architecture: RequestUpdateSkillSlot3Layer() instead
	//  * Handles network authority, logging, and transaction boundaries
	//  * @param PlayerIdentity Target player identity containing SkillComponent
	//  * @param SlotIndex Slot index to unregister
	//  */
	// UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: RequestUpdateSkillSlot3Layer() instead")
	// void RequestUnregisterSkill(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 SlotIndex);
	//
	// /**
	//  * DEPRECATED: Request skill slot swap with full validation pipeline
	//  * Use 3-Layer Mapping Architecture: RequestSavePlayerSkills3Layer() instead
	//  * Handles network authority, logging, and transaction boundaries
	//  * @param PlayerIdentity Target player identity containing SkillComponent
	//  * @param SlotIndexA First slot index
	//  * @param SlotIndexB Second slot index
	//  */
	// UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: RequestSavePlayerSkills3Layer() instead")
	// void RequestSwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 SlotIndexA, int32 SlotIndexB);
	//
	// /**
	//  * DEPRECATED: Request player skill loading with full validation pipeline
	//  * Use 3-Layer Mapping Architecture: RequestLoadPlayerSkills3Layer() instead
	//  * Handles network authority, logging, and transaction boundaries
	//  * @param PlayerIdentity Target player identity containing SkillComponent
	//  */
	// UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: RequestLoadPlayerSkills3Layer() instead")
	// void RequestLoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);
	//
	// /**
	//  * DEPRECATED: Request player skill saving with full validation pipeline
	//  * Use 3-Layer Mapping Architecture: RequestSavePlayerSkills3Layer() instead
	//  * Handles network authority, logging, and transaction boundaries
	//  * @param PlayerIdentity Target player identity containing SkillComponent
	//  * @param SkillData The skill domain data to save
	//  */
	// UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: RequestSavePlayerSkills3Layer() instead")
	// void RequestSavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FSkillDomain& SkillData);
	//
	// /**
	//  * DEPRECATED: Request skill cooldown update with full validation pipeline
	//  * Use 3-Layer Mapping Architecture: RequestSavePlayerSkills3Layer() instead
	//  * Handles network authority, logging, and transaction boundaries
	//  * @param PlayerIdentity Target player identity containing SkillComponent
	//  * @param SlotIndex Slot index
	//  * @param LastUsedTime When the skill was last used
	//  * @param RemainingCooldown Remaining cooldown time
	//  */
	// UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: RequestSavePlayerSkills3Layer() instead")
	// void RequestUpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown);

public:
	// Delegate for notifying when skill data loading is completed
	UPROPERTY(BlueprintAssignable)
	FOnSkillDataLoadCompleted OnSkillDataLoadCompleted;

private:
	// Callback function for when player skills are loaded
	UFUNCTION()
	void OnPlayerSkillsLoaded(const FGuid& PlayerGuid);

	// Repository interface for Dependency Injection
	UPROPERTY()
	TScriptInterface<ISkillRepositoryInterface> SkillRepositoryInterface;

	// Default concrete implementation
	UPROPERTY()
	USkillRepository* DefaultSkillRepository;

	UPROPERTY()
	TObjectPtr<USkillDomainService> DomainService;
};
