#include "Shared/Player/GGwaPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "AuthModule/Public/AuthSubsystem.h"
#include "Engine/World.h"
#include "Shared/GameState/GGwaGameState.h"

AGGwaPlayerController::AGGwaPlayerController() {

}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
	
	if (IsLocalController()){
		if (auto ClientManagerInterface = Cast<AGGwaGameState>(GetWorld()->GetGameState())->GetClientManagerInterface()) {
			CachedClientManagerInterface = ClientManagerInterface;
		}
	}
}

void AGGwaPlayerController::AcknowledgePossession(class APawn* PossessedPawn) {
	Super::AcknowledgePossession(PossessedPawn);
	AGGwaCharacter * MyCharacter = Cast<AGGwaCharacter>(PossessedPawn);
	if (nullptr != MyCharacter) {
		UE_LOG(LogTemp,Warning,TEXT("AGGwaPlayerController::AcknowledgePossession : Pawn Possessed"));
		UGGwaAbilitySystemComponent * ASC = Cast<UGGwaAbilitySystemComponent>( GetPlayerState<AGGwaPlayerState>()->GetAbilitySystemComponent());
		if (ASC) {
			ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds("Confirm", "Cancel", FTopLevelAssetPath(TEXT("/Script/SkillModule"), TEXT("EAbilityInputID"))));
		}
	}
}

void AGGwaPlayerController::Server_InitiateReward_Implementation(const FString& PlayerId, const FRewardRequest& Payload) {
	IServerLogicBridge* Bridge = Cast<IServerLogicBridge>(GetWorld()->GetSubsystem<UWorldSubsystem>());
	// Bridge->InitiateRewardFlow(PlayerId, Payload, FOnFlowComplete::CreateUObject(this, &AGGwaPlayerController::Client_OnRewardResult));
}


// ============================================================================
// AUTHENTICATION RPC INTERFACES
// ============================================================================

void AGGwaPlayerController::RequestServerRegistration(const FString& Username, const FString& Password) {
	Server_Register_Implementation(Username, Password);
}

void AGGwaPlayerController::RequestServerLogin(const FString& Username, const FString& Password) {
	Server_Login_Implementation(Username, Password);
}

void AGGwaPlayerController::Request_Client_TravelToGameWorld(const FString& MapURL) {
	Client_TravelToGameWorld_Implementation(MapURL);
}

bool AGGwaPlayerController::IsAuthRPCAvailable() const {
	return true;
}

// ============================================================================
// AUTHENTICATION RPC IMPLEMENTATIONS
// ============================================================================

void AGGwaPlayerController::Server_Register_Implementation(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Server_Register: Registration request for username: %s"), *Username);

	FString ClientIP = TEXT("Unknown");
	if (GetNetConnection() && GetNetConnection()->RemoteAddr.IsValid())
	{
		ClientIP = GetNetConnection()->RemoteAddr->ToString(false);
	}

	if (UAuthSubsystem* AuthSubsystem = GetGameInstance()->GetSubsystem<UAuthSubsystem>())
	{
		AuthSubsystem->OnServerRegistrationComplete.AddDynamic(this, &AGGwaPlayerController::Client_OnRegistrationResult);
		AuthSubsystem->RequestServerRegistration(Username, Password, ClientIP);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaPlayerController::Server_Register: AuthSubsystem not found"));
	}
}

bool AGGwaPlayerController::Server_Register_Validate(const FString& Username, const FString& Password){
	return !Username.IsEmpty() && !Password.IsEmpty() && Username.Len() <= 30 && Password.Len() <= 128;
}

void AGGwaPlayerController::Server_Login_Implementation(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Server_Login: Login request for username: %s"), *Username);

	// Get client IP for audit logging
	FString ClientIP = TEXT("Unknown");
	if (GetNetConnection() && GetNetConnection()->RemoteAddr.IsValid())
	{
		ClientIP = GetNetConnection()->RemoteAddr->ToString(false);
	}

	// Get AuthSubsystem and delegate the request
	if (UAuthSubsystem* AuthSubsystem = GetGameInstance()->GetSubsystem<UAuthSubsystem>())
	{
		// Bind to AuthSubsystem events to get the result
		AuthSubsystem->OnServerAuthenticationComplete.AddDynamic(this, &AGGwaPlayerController::Client_OnLoginResult);

		// Send login request to AuthSubsystem
		AuthSubsystem->RequestServerAuthentication(Username, Password, ClientIP, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaPlayerController::Server_Login: AuthSubsystem not found"));
	}
}

bool AGGwaPlayerController::Server_Login_Validate(const FString& Username, const FString& Password)
{
	// Basic validation - AuthSubsystem will do detailed validation
	return !Username.IsEmpty() && !Password.IsEmpty() && Username.Len() <= 30 && Password.Len() <= 128;
}

void AGGwaPlayerController::Client_OnRegistrationResult_Implementation(bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_OnRegistrationResult: Success=%s, Message=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *Message);

	// Use ClientServiceManager for interface method delegation
}

void AGGwaPlayerController::Client_OnLoginResult_Implementation(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_OnLoginResult: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);
}

void AGGwaPlayerController::Client_TravelToGameWorld_Implementation(const FString& MapURL)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_TravelToGameWorld: Traveling to %s"), *MapURL);
	
#if !UE_SERVER
	// Perform client travel to the game world
	ClientTravel(MapURL, TRAVEL_Relative);
	// Note: UIManagerSubsystem access would need to be handled through interface if needed
#endif
}

// ============================================================================
// IClientComponentProvider INTERFACE IMPLEMENTATION
// All calls delegated to UIManagerInterface (interface-based architecture)
// ============================================================================

TScriptInterface<IClientManagerInterface> AGGwaPlayerController::GetUIManagerInterface() {
	return CachedClientManagerInterface;
}

void AGGwaPlayerController::InitializeUI(const USkillComponent* SkillComponent) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->InitializeUI(SkillComponent);
}

void AGGwaPlayerController::ProcessRegistration(const FString& Username, const FString& Password) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->ProcessRegistration(Username, Password);
}

void AGGwaPlayerController::ProcessLogin(const FString& Username, const FString& Password) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->ProcessLogin(Username, Password);
}

void AGGwaPlayerController::HandleRegistrationResult(bool bSuccess, const FString& Message) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->HandleRegistrationResult(bSuccess, Message);
}

void AGGwaPlayerController::HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->HandleLoginResult(bSuccess, Token, UserId);
}

void AGGwaPlayerController::ProcessMouseOverDetection() {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->ProcessMouseOverDetection();
}

void AGGwaPlayerController::NotifyStateChanged() {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->NotifyStateChanged();
}

void AGGwaPlayerController::ProcessBossData_Implementation(const FBossDataStruct& BossData) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->ProcessBossData(BossData);
}

void AGGwaPlayerController::ProcessSkillData_Implementation(const USkillComponent* SkillComponent) {
	if (!IsLocalController()) return;
	CachedClientManagerInterface->ProcessSkillData(SkillComponent);
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	if (!IsLocalController()) return;
	Super::PlayerTick(DeltaTime);
	ProcessMouseOverDetection();
}
