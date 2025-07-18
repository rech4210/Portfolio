// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "AuthModule/Public/AuthSubsystem.h"

AGGwaPlayerController::AGGwaPlayerController() {
	// Initialize client component provider placeholder
	ClientComponentProvider = nullptr;
}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	
	// Server-client logging
	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
	
}

void AGGwaPlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& BossCharacter) {
	// Use interface method instead of direct component access
	ReceiveBossDataFromServer(BossCharacter);
	// Always broadcast to any bound delegates
	OnBossDataReceived.Broadcast(BossCharacter);
	UE_LOG(LogTemp, Log, TEXT("BossData Called From Server"));
}

void AGGwaPlayerController::Client_ReceiveSkillData_Implementation(const USkillComponent* SkillComponent) {
	// Use interface method instead of direct component access
	ReceiveSkillDataFromServer(SkillComponent);
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


void AGGwaPlayerController::Server_InitiateReward_Implementation(const FString& PlayerId, const FRewardRequest& Payload) {
	IServerLogicBridge* Bridge = Cast<IServerLogicBridge>(GetWorld()->GetSubsystem<UWorldSubsystem>());
	// Bridge->InitiateRewardFlow(PlayerId, Payload, FOnFlowComplete::CreateUObject(this, &AGGwaPlayerController::Client_OnRewardResult));
}

// ============================================================================
// AUTHENTICATION RPC IMPLEMENTATIONS
// ============================================================================

void AGGwaPlayerController::Server_Register_Implementation(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Server_Register: Registration request for username: %s"), *Username);

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
		AuthSubsystem->OnServerRegistrationComplete.AddDynamic(this, &AGGwaPlayerController::OnAuthSubsystemRegistrationComplete);

		// Send registration request to AuthSubsystem
		AuthSubsystem->RequestServerRegistration(Username, Password, ClientIP);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaPlayerController::Server_Register: AuthSubsystem not found"));
		Client_OnRegistrationResult(false, TEXT("Authentication service unavailable"));
	}
}

bool AGGwaPlayerController::Server_Register_Validate(const FString& Username, const FString& Password)
{
	// Basic validation - AuthSubsystem will do detailed validation
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
		AuthSubsystem->OnServerAuthenticationComplete.AddDynamic(this, &AGGwaPlayerController::OnAuthSubsystemAuthenticationComplete);

		// Send login request to AuthSubsystem
		AuthSubsystem->RequestServerAuthentication(Username, Password, ClientIP, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaPlayerController::Server_Login: AuthSubsystem not found"));
		Client_OnLoginResult(false, TEXT(""), TEXT(""));
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

	// Use interface method instead of direct component access
	OnServerRegistrationResult(bSuccess, Message);
}

void AGGwaPlayerController::Client_OnLoginResult_Implementation(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_OnLoginResult: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

	// Use interface method instead of direct component access
	OnServerLoginResult(bSuccess, Token, UserId);
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
// AuthSubsystem Event Handlers
// ============================================================================

void AGGwaPlayerController::OnAuthSubsystemRegistrationComplete(bool bSuccess, const FString& Message)
{
	// Forward the result to the client
	Client_OnRegistrationResult_Implementation(bSuccess, Message);
}

void AGGwaPlayerController::OnAuthSubsystemAuthenticationComplete(bool bSuccess, const FString& Token, const FString& UserId)
{
	// Forward the result to the client
	Client_OnLoginResult_Implementation(bSuccess, Token, UserId);

	// Note: ClientTravel is now handled by AuthSubsystem after game data loading
	// This prevents duplicate travel calls and ensures data is loaded before travel
}
// void AGGwaPlayerController::Client_OnRewardResult_Implementation(bool bOK, const FRewardData& Data,const FString& Error) {
// }

// --- Client-only functions ---

void AGGwaPlayerController::InitClientWidget(const USkillComponent* SkillComponent) {
	// Use interface method instead of direct component access
	InitializeClientUI(SkillComponent);
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// Use interface method instead of direct component access
	HandleClientMouseOverDetection();
}

void AGGwaPlayerController::OnLoginSuccess(const FString& Token) {
	// Implementation for login success
}

void AGGwaPlayerController::OnLoginFailure(const FString& ErrorReason) {
	// Implementation for login failure
}

// ============================================================================
// LOGIN UI FUNCTIONALITY (Integrated from LoginPlayerController)
// ============================================================================

void AGGwaPlayerController::RequestRegistration(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::RequestRegistration: Username=%s"), *Username);

	// Use interface method instead of direct component access
	RequestClientRegistration(Username, Password);
}

void AGGwaPlayerController::RequestLogin(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::RequestLogin: Username=%s"), *Username);

	// Use interface method instead of direct component access
	RequestClientLogin(Username, Password);
}

// ============================================================================
// IClientComponentProvider INTERFACE IMPLEMENTATION
// ============================================================================

void AGGwaPlayerController::InitializeClientAuth() {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("InitializeClientAuth: Default implementation called"));
}

void AGGwaPlayerController::RequestClientRegistration(const FString& Username, const FString& Password) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("RequestClientRegistration: Default implementation called"));
}

void AGGwaPlayerController::RequestClientLogin(const FString& Username, const FString& Password) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("RequestClientLogin: Default implementation called"));
}

void AGGwaPlayerController::OnServerRegistrationResult(bool bSuccess, const FString& Message) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("OnServerRegistrationResult: Default implementation called"));
}

void AGGwaPlayerController::OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("OnServerLoginResult: Default implementation called"));
}

void AGGwaPlayerController::InitializeClientUI(const USkillComponent* SkillComponent) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("InitializeClientUI: Default implementation called"));
}

void AGGwaPlayerController::HandleClientMouseOverDetection() {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("HandleClientMouseOverDetection: Default implementation called"));
}

void AGGwaPlayerController::NotifyClientStateChanged() {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("NotifyClientStateChanged: Default implementation called"));
}

void AGGwaPlayerController::ReceiveBossDataFromServer(const FBossDataStruct& BossData) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("ReceiveBossDataFromServer: Default implementation called"));
}

void AGGwaPlayerController::ReceiveSkillDataFromServer(const USkillComponent* SkillComponent) {
	// Default implementation - override in client module
	UE_LOG(LogTemp, Warning, TEXT("ReceiveSkillDataFromServer: Default implementation called"));
}

