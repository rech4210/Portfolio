// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/PVPModeManager.h"
void UPVPModeManager::InitMode() {
	Super::InitMode();
}

void UPVPModeManager::StartBattle() {
	Super::StartBattle();
}

void UPVPModeManager::Tick(float DeltaTime) {
}

void UPVPModeManager::EndBattle() {
	Super::EndBattle();
}

void UPVPModeManager::ResetBattle() {
	Super::ResetBattle();
}

TStatId UPVPModeManager::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPVPModeManager, STATGROUP_Tickables);
}
