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
	// Use Case Orchestration - App Layer Responsibilities Only
	// ============================================================================

	/**
	 * Request skill registration with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SkillData Skill data to register
	 */
	void RequestRegisterSkill(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillDataAsset* SkillData);

	/**
	 * Request skill unregistration with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SlotId Slot ID to unregister
	 */
	void RequestUnregisterSkill(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotId);

	/**
	 * Request skill slot swap with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SlotIdA First slot ID
	 * @param SlotIdB Second slot ID
	 */
	void RequestSwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotIdA, const FGuid& SlotIdB);

	/**
	 * Request player skill loading with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing SkillComponent
	 */
	void RequestLoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);

	/**
	 * Request player skill saving with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SkillData The skill domain data to save
	 */
	void RequestSavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FSkillDomain& SkillData);

	/**
	 * Request skill cooldown update with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing SkillComponent
	 * @param SlotId Slot ID
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 */
	void RequestUpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown);

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
