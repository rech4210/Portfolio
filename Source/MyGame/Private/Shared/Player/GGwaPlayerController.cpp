// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/Service/ClientServiceManager.h"
#include "AuthModule/Public/AuthSubsystem.h"

AGGwaPlayerController::AGGwaPlayerController() {
	// Create ClientServiceManager instance
	ClientServiceManager = CreateDefaultSubobject<UClientServiceManager>(TEXT("ClientServiceManager"));
}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	
	// Server-client logging
	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
	
}

void AGGwaPlayerController::InitClientWidget(const USkillComponent* SkillComponent) {
}


void AGGwaPlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& BossCharacter) {
}

void AGGwaPlayerController::Client_ReceiveSkillData_Implementation(const USkillComponent* SkillComponent) {
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
		AuthSubsystem->OnServerRegistrationComplete.AddDynamic(this, &AGGwaPlayerController::OnAuthSubsystemRegistrationComplete);
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
		AuthSubsystem->OnServerAuthenticationComplete.AddDynamic(this, &AGGwaPlayerController::OnAuthSubsystemAuthenticationComplete);

		// Send login request to AuthSubsystem
		AuthSubsystem->RequestServerAuthentication(Username, Password, ClientIP, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaPlayerController::Server_Login: AuthSubsystem not found"));
		// Client_OnLoginResult(false, TEXT(""), TEXT(""));
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
	if (ClientServiceManager)
	{
		ClientServiceManager->OnServerRegistrationResult(bSuccess, Message);
	}
}

void AGGwaPlayerController::Client_OnLoginResult_Implementation(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_OnLoginResult: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

	// Use ClientServiceManager for interface method delegation
	if (ClientServiceManager)
	{
		ClientServiceManager->OnServerLoginResult(bSuccess, Token, UserId);
	}
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

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// Delegate to ClientServiceManager instead of direct component access
	if (ClientServiceManager && IsLocalController())
	{
		ClientServiceManager->HandleClientMouseOverDetection();
	}
}

// ============================================================================
// CLIENT SERVICE REGISTRATION (GGwaGameState pattern)
// ============================================================================

void AGGwaPlayerController::RegisterClientAuthService(TScriptInterface<IClientAuthInterface> AuthService)
{
	if (ClientServiceManager)
	{
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController: Registering AuthService to ClientServiceManager"));
		ClientServiceManager->SetAuthService(AuthService);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController: Cannot register AuthService - ClientServiceManager not created"));
	}
}

void AGGwaPlayerController::RegisterClientUIService(TScriptInterface<IClientUIInterface> UIService)
{
	if (ClientServiceManager)
	{
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController: Registering UIService to ClientServiceManager"));
		ClientServiceManager->SetUIService(UIService);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController: Cannot register UIService - ClientServiceManager not created"));
	}
}

// ============================================================================
// IClientComponentProvider INTERFACE IMPLEMENTATION
// All calls delegated to ClientService
// ============================================================================

// Authentication component interface
void AGGwaPlayerController::InitializeClientAuth() {
	if (ClientServiceManager)
	{
		ClientServiceManager->InitializeClientAuth();
	}
}

void AGGwaPlayerController::RequestClientRegistration(const FString& Username, const FString& Password) {
	if (ClientServiceManager)
	{
		ClientServiceManager->RequestClientRegistration(Username, Password);
	}
}

void AGGwaPlayerController::RequestClientLogin(const FString& Username, const FString& Password) {
	if (ClientServiceManager)
	{
		ClientServiceManager->RequestClientLogin(Username, Password);
	}
}

void AGGwaPlayerController::OnServerRegistrationResult(bool bSuccess, const FString& Message) {
	if (ClientServiceManager)
	{
		ClientServiceManager->OnServerRegistrationResult(bSuccess, Message);
	}
}

void AGGwaPlayerController::OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) {
	if (ClientServiceManager)
	{
		ClientServiceManager->OnServerLoginResult(bSuccess, Token, UserId);
	}
}


// UI component interface

void AGGwaPlayerController::InitializeClientUI(const USkillComponent* SkillComponent) {
	if (ClientServiceManager)
	{
		ClientServiceManager->InitializeClientUI(SkillComponent);
	}
}

void AGGwaPlayerController::HandleClientMouseOverDetection() {
	if (ClientServiceManager)
	{
		ClientServiceManager->HandleClientMouseOverDetection();
	}
}

void AGGwaPlayerController::NotifyClientStateChanged() {
	if (ClientServiceManager)
	{
		ClientServiceManager->NotifyClientStateChanged();
	}
}

void AGGwaPlayerController::ReceiveBossDataFromServer(const FBossDataStruct& BossData) {
	if (ClientServiceManager)
	{
		ClientServiceManager->ReceiveBossDataFromServer(BossData);
	}
}

void AGGwaPlayerController::ReceiveSkillDataFromServer(const USkillComponent* SkillComponent) {
	if (ClientServiceManager)
	{
		ClientServiceManager->ReceiveSkillDataFromServer(SkillComponent);
	}
}

