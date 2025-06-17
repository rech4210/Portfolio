
#include "GameMode/BattleFlowController.h"
#include "GameMode/PVPModeManager.h"
#include "GameMode/RaidModeManager.h"

void UBattleFlowController::InitBattleMode(EModeType ModeType) {
	SetupBattleMode(ModeType);
	BattleMode->InitMode();
}

void UBattleFlowController::SetupBattleMode(EModeType ModeType) {
	switch (ModeType) {
	case EModeType::Raid:
		BattleMode = NewObject<URaidModeManager>(this);
		UE_LOG(LogTemp, Log, TEXT("Raid mode initialized"));
		break;
	case EModeType::PVP:
		BattleMode = NewObject<UPVPModeManager>(this);
		UE_LOG(LogTemp, Log, TEXT("PVP mode initialized"));
		break;
	case EModeType::None:
		UE_LOG(LogTemp, Log, TEXT("mode NotSetted"));
		break;
	}
}
