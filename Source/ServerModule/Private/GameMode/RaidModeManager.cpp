

#include "GameMode/RaidModeManager.h"


void URaidModeManager::InitMode() {
	Super::InitMode();
}

void URaidModeManager::StartBattle() {
}


void URaidModeManager::Tick(float DeltaTime) {
}

void URaidModeManager::EndBattle() {
}

void URaidModeManager::ResetBattle() {
}

TStatId URaidModeManager::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(URaidModeManager, STATGROUP_Tickables);
}
