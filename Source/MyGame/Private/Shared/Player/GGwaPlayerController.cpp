// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Player/GGwaPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
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

	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
}

void AGGwaPlayerController::Client_ApplyAbilityDataAsset_Implementation(UBaseDataAsset* Data) {
	UE_LOG(LogTemp, Log, TEXT("ApplyAbility Data Called From Server"));
}

void AGGwaPlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& BossCharacter) {
	UE_LOG(LogTemp, Log, TEXT("BossData Called From Server"));
}

void AGGwaPlayerController::AcknowledgePossession(class APawn* PossessedPawn) {
	Super::AcknowledgePossession(PossessedPawn);
	AGGwaCharacter * MyCharacter = Cast<AGGwaCharacter>(PossessedPawn);
	if (nullptr != MyCharacter) {
		UE_LOG(LogTemp,Warning,TEXT("AGGwaPlayerController::AcknowledgePossession : Pawn Possessed"));
		UGGwaAbilitySystemComponent * ASC = Cast<UGGwaAbilitySystemComponent>( GetPlayerState<AGGwaPlayerState>()->GetAbilitySystemComponent());
		if (ASC) {
			ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds("Confirm", "Cancel", FTopLevelAssetPath(TEXT("/Script/MyGame"), TEXT("EAbilityInputID"))));
		}
	}
}

void AGGwaPlayerController::Server_InitiateReward_Implementation(const FString& PlayerId, const FRewardRequest& Payload) {
	IServerLogicBridge* Bridge = Cast<IServerLogicBridge>(GetWorld()->GetSubsystem<UWorldSubsystem>());
	// Bridge->InitiateRewardFlow(PlayerId, Payload, FOnFlowComplete::CreateUObject(this, &AGGwaPlayerController::Client_OnRewardResult));
}

// void AGGwaPlayerController::Client_OnRewardResult_Implementation(bool bOK, const FRewardData& Data,const FString& Error) {
	
// }

