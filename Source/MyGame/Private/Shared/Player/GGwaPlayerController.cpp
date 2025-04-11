// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Player/GGwaPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"

AGGwaPlayerController::AGGwaPlayerController() {
}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	bShowMouseCursor = true;
	FInputModeGameAndUI inputMode;
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	inputMode.SetHideCursorDuringCapture(false);
	SetInputMode(inputMode);

	if (IsLocalController()) {
		InitClientWidget();
	}
	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
}

void AGGwaPlayerController::AcknowledgePossession(class APawn* PossessedPawn) {
	Super::AcknowledgePossession(PossessedPawn);
	AGGwaCharacter * MyCharacter = Cast<AGGwaCharacter>(PossessedPawn);
	if (nullptr != MyCharacter) {
		// Do something
		UE_LOG(LogTemp,Warning,TEXT("AGGwaPlayerController::AcknowledgePossession : Pawn Possessed"));
		UAbilitySystemComponent * ASC = GetPlayerState<AGGwaPlayerState>()->GetAbilitySystemComponent();
		if (ASC) {
			ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds("Confirm", "Cancel", FTopLevelAssetPath(TEXT("/Script/MyGame"), TEXT("EAbilityInputID"))));
		}
	}
}
