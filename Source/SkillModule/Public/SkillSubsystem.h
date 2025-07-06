// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "SkillSubsystem.generated.h"

class USkillConfigRepository;
class USkillStateRepository;
class APlayerState;
class USkillComponent;

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<ISkillConfigRepositoryInterface> GetSkillConfigRepository() const;
	TScriptInterface<ISkillStateRepositoryInterface> GetSkillStateRepository() const;

	/** Entry point for loading player skill data. Can be called from PlayerState's BeginPlay. */
	void RequestLoadSkillData(APlayerState* PlayerState);

	/** Called by the SkillComponent on clients when skill data is replicated. */
	void Client_OnSkillStateUpdated(USkillComponent* SkillComponent);

private:
	UPROPERTY()
	USkillConfigRepository* SkillConfigRepository;

	UPROPERTY()
	USkillStateRepository* SkillStateRepository;
};
