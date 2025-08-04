#include "Shared/Mode/BaseGameMode.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"

ABaseGameMode::ABaseGameMode()
{
	PlayerControllerClass = AGGwaPlayerController::StaticClass();
	PlayerStateClass    = AGGwaPlayerState::StaticClass();
	DefaultPawnClass    = AGGwaCharacter::StaticClass();
}

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) {
	}
	InitializeServerManagers();
}

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("PostLogin: NewPlayer is null"));
		return;
	}

	if (AGGwaPlayerState* State = NewPlayer->GetPlayerState<AGGwaPlayerState>())
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerState found: %s"), *State->GetPlayerName());
	}

	if (NewPlayer->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player already has Pawn"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Pawn not spawned yet"));
	}
}

void ABaseGameMode::HandleAbilityActivated(const AActor* InstigatorActor,
										   FName AbilityName,
										   const FVector& TargetLocation)
{
	UE_LOG(LogTemp, Log, TEXT("[Ability] %s activated %s at %s"),
		   *InstigatorActor->GetName(),
		   *AbilityName.ToString(),
		   *TargetLocation.ToString());
}
