// @Needmodifi
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IEquipmentRepositoryInterface.h"
#include "Interface/PlayerIdentityInterface.h"
#include "EquipmentSubsystem.generated.h"

class UEquipmentRepository;
class UEquipmentComponent;

/**
 * 
 */
UCLASS()
class EQUIPMENTMODULE_API UEquipmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<IEquipmentRepositoryInterface> GetEquipmentRepository() const;

	/** Entry point for loading equipment data. Can be called from PlayerState's BeginPlay. */
	void RequestLoadEquipmentData(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);

	/** Called by the EquipmentComponent on clients when equipment data is replicated. */
	void Client_OnEquipmentStateUpdated(UEquipmentComponent* EquipmentComponent);

private:
	UPROPERTY()
	UEquipmentRepository* EquipmentRepository;
};
