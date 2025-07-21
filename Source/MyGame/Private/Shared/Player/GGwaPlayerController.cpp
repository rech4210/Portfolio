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
	UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - IsServer: %d | IsLocallyControlled: %d"), 
		HasAuthority(), IsLocalController());

#if !UE_SERVER
	if (IsLocalController())
	{
		// Step 1: Check World validity
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("- Client Only Controller::BeginPlay - World is null!"));
			return;
		}
		if (ClientAuthComponentClass && ClientUIComponentClass) {
			ClientAuthComponent = NewObject<UActorComponent>(this, ClientAuthComponentClass);
			Cast<IClientAuthInterface>(ClientAuthComponent)->InitializeAuth();
			CachedClientManagerInterface = Cast<IClientAuthInterface>(ClientAuthComponent)->GetClientSubSystem();
			
			RegistClientComponent(ClientAuthComponent);
			ClientUIComponent = NewObject<UActorComponent>(this, ClientUIComponentClass);
			// Cast<IClientUIInterface>(ClientUIComponent)->InitializeUI();
			RegistClientComponent(ClientUIComponent);
		}
	}
#endif
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

IClientManagerInterface* AGGwaPlayerController::GetUIManagerInterface() {
	return CachedClientManagerInterface;
}

void AGGwaPlayerController::RegistClientComponent(UActorComponent* Component) {
	CachedClientManagerInterface->RegistClientComponent(Component);
}

void AGGwaPlayerController::InitializeUI(const USkillComponent* SkillComponent) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->InitializeUI(SkillComponent);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::InitializeUI - Delegated to ClientManager"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::InitializeUI - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::ProcessRegistration(const FString& Username, const FString& Password) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->ProcessRegistration(Username, Password);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::ProcessRegistration - Delegated for user: %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::ProcessRegistration - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::ProcessLogin(const FString& Username, const FString& Password) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->ProcessLogin(Username, Password);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::ProcessLogin - Delegated for user: %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::ProcessLogin - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::HandleRegistrationResult(bool bSuccess, const FString& Message) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->HandleRegistrationResult(bSuccess, Message);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::HandleRegistrationResult - Success: %s"), 
			bSuccess ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::HandleRegistrationResult - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->HandleLoginResult(bSuccess, Token, UserId);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::HandleLoginResult - Success: %s, UserId: %s"), 
			bSuccess ? TEXT("true") : TEXT("false"), *UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::HandleLoginResult - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::ProcessMouseOverDetection() {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->ProcessMouseOverDetection();
		// No log here as this is called frequently
	}
	// No warning log for mouse detection as it's called frequently
}

void AGGwaPlayerController::NotifyStateChanged() {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->NotifyStateChanged();
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::NotifyStateChanged - Delegated"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::NotifyStateChanged - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::ProcessBossData_Implementation(const FBossDataStruct& BossData) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->ProcessBossData(BossData);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::ProcessBossData - Delegated boss data"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::ProcessBossData - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::ProcessSkillData_Implementation(const USkillComponent* SkillComponent) {
	if (!IsLocalController()) return;
	
	if (CachedClientManagerInterface)
	{
		CachedClientManagerInterface->ProcessSkillData(SkillComponent);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::ProcessSkillData - Delegated skill data"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::ProcessSkillData - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	if (!IsLocalController()) return;
	Super::PlayerTick(DeltaTime);
	ProcessMouseOverDetection();
}
