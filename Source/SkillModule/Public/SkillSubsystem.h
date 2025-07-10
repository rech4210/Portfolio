// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "SkillSubsystem.generated.h"

class USkillConfigRepository;
class USkillStateRepository;
class USkillRepository;
class USkillDomainService;
class APlayerState;
class USkillComponent;

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
	USkillDomainService* CreateDomainService();

	// Legacy support methods
	TScriptInterface<ISkillConfigRepositoryInterface> GetSkillConfigRepository() const;
	TScriptInterface<ISkillStateRepositoryInterface> GetSkillStateRepository() const;

	/**
	 * Legacy support: Entry point for loading player skill data
	 * @deprecated Use SkillDomainService instead
	 */
	void RequestLoadSkillData(APlayerState* PlayerState);

	/**
	 * Legacy support: Called by the SkillComponent on clients when skill data is replicated
	 * @deprecated Use SkillDomainService instead
	 */
	void Client_OnSkillStateUpdated(USkillComponent* SkillComponent);

private:
	// Repository interface for Dependency Injection
	UPROPERTY()
	TScriptInterface<ISkillRepositoryInterface> SkillRepositoryInterface;

	// Default concrete implementation
	UPROPERTY()
	USkillRepository* DefaultSkillRepository;

	// Legacy repositories
	UPROPERTY()
	USkillConfigRepository* SkillConfigRepository;

	UPROPERTY()
	USkillStateRepository* SkillStateRepository;
};
