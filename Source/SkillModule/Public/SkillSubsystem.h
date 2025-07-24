// Fill out your copyright notice in the Description page of Project Settings.

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
	// 3-LAYER MAPPING ARCHITECTURE USE CASES (RECOMMENDED)
	// ============================================================================

	/**
	 * Request player skill loading using 3-layer mapping architecture
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to load skills for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 */
	void RequestLoadPlayerSkills3Layer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 UserId, const FString& SlotKey);

	/**
	 * Request player skill saving using 3-layer mapping architecture
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to save skills for
	 */
	void RequestSavePlayerSkills3Layer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 UserId);

	/**
	 * Request skill slot update using 3-layer mapping architecture
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to update skills for
	 * @param SlotIndex Slot index to update
	 * @param SkillData New skill data for the slot (null to unregister)
	 */
	void RequestUpdateSkillSlot3Layer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 UserId, int32 SlotIndex, USkillDataAsset* SkillData);

	/**
	 * Request skill cooldown update using 3-layer mapping architecture
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to update cooldown for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 * @param SlotIndex Slot index to update
	 * @param LastUsedTime When the skill was last used
	 */
	void RequestUpdateSkillCooldown3Layer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime);

	/**
	 * Request clearing all skill slots using 3-layer mapping architecture
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to clear slots for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 */
	void RequestClearPlayerSkills3Layer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 UserId, const FString& SlotKey);

	/**
	 * Request skill swap using 3-layer mapping architecture
	 * @param PlayerIdentity Target player identity containing SkillComponent
	 * @param UserId User ID to swap skills for
	 * @param SlotIndexA First slot index
	 * @param SlotIndexB Second slot index
	 */
	void RequestSwapSkillSlots3Layer(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, int32 UserId, int32 SlotIndexA, int32 SlotIndexB);

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

private:
	// Repository interface for Dependency Injection
	UPROPERTY()
	TScriptInterface<ISkillRepositoryInterface> SkillRepositoryInterface;

	// Default concrete implementation
	UPROPERTY()
	USkillRepository* DefaultSkillRepository;

	UPROPERTY()
	TObjectPtr<USkillDomainService> DomainService;
};
