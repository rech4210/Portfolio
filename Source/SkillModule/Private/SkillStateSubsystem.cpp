// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillStateSubsystem.h"
#include "Repositories/SkillStateRepository.h"

void USkillStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SkillStateRepository = NewObject<USkillStateRepository>(this, TEXT("SkillStateRepository"));
	DomainService = NewObject<USkillDomainService>(this, TEXT("SkillDomainService"));
}

void USkillStateSubsystem::Deinitialize()
{
	SkillStateRepository = nullptr;
	Super::Deinitialize();
}

TScriptInterface<ISkillStateRepositoryInterface> USkillStateSubsystem::GetSkillStateRepository() const
{
	return SkillStateRepository;
}

USkillDomainService* USkillStateSubsystem::GetDomainService() const {
	return DomainService;
}
