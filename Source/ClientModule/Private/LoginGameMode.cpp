#include "LoginGameMode.h"
#include "Player/LoginPlayerController.h"
#include "Shared/Player/GGwaPlayerController.h"

constexpr static float GAME_MODE_FREQUENCY = 3.0f;
ALoginGameMode::ALoginGameMode() {
	PlayerControllerClass = AGGwaPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void ALoginGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	static float LogTimer = 0.0f;
	LogTimer += DeltaSeconds;
	if (LogTimer >= GAME_MODE_FREQUENCY)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client is ALIVE. Current GameMode is [ %s ] running in map [ %s ]"), *GetName(), *GetWorld()->GetName());
		LogTimer = 0.0f;
	}
}