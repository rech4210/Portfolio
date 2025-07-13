// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillDomainService.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "SkillStateSubsystem.generated.h"

class USkillStateRepository;

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	TScriptInterface<ISkillStateRepositoryInterface> GetSkillStateRepository() const;
	USkillDomainService* GetDomainService() const;
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkillDomainService> DomainService;
	UPROPERTY()
	USkillStateRepository* SkillStateRepository;
};
