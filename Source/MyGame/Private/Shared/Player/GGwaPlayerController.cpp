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
	UE_LOG(LogTemp, Warning, TEXT("=== GGwaPlayerController::BeginPlay DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("Controller Address: %p | IsServer: %d | IsLocalPlayerController: %d"), 
		this, HasAuthority(), IsLocalPlayerController());

#if !UE_SERVER
	if (IsLocalPlayerController())
	{
		// Step 1: Check World validity
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("- Client Only Controller::BeginPlay - World is null!"));
			return;
		}
		
		// Step 2: Check for cached token/userid from previous instance
		if (CachedAuthToken.IsEmpty() || CachedUserId.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - No cached credentials, this may be initial login"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - Found cached credentials: UserId=%s"), *CachedUserId);
		}
		
		// Step 3: Check if components are already cached (prevents re-initialization after map travel)
		if (CachedClientManagerInterface.GetInterface() != nullptr && 
			ClientAuthComponent != nullptr && 
			ClientUIComponent != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - Components already cached, validating..."));
			
			// Validate cached interface is still functional
			if (CachedClientManagerInterface.GetObject() && IsValid(CachedClientManagerInterface.GetObject()))
			{
				UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - Cached interface still valid, skipping re-initialization"));
				return;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - Cached interface invalid, forcing re-initialization"));
				// Clear invalid cache
				CachedClientManagerInterface = TScriptInterface<IClientManagerInterface>();
				ClientAuthComponent = nullptr;
				ClientUIComponent = nullptr;
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - Initializing components..."));
		
		if (ClientAuthComponentClass && ClientUIComponentClass) {
			ClientAuthComponent = NewObject<UActorComponent>(this, ClientAuthComponentClass);
			
			// Initialize auth component
			if (auto AuthInterface = Cast<IClientAuthInterface>(ClientAuthComponent))
			{
				AuthInterface->InitializeAuth();
				
				// Get and cache the client subsystem properly
				TScriptInterface<IClientManagerInterface> SubSystem = AuthInterface->GetClientSubSystem();
				if (SubSystem.GetInterface())
				{
					CachedClientManagerInterface = SubSystem;
					UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - CachedClientManagerInterface successfully assigned"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController::BeginPlay - Failed to get ClientSubSystem"));
				}
			}
			
			RegistClientComponent(ClientAuthComponent);
			ClientUIComponent = NewObject<UActorComponent>(this, ClientUIComponentClass);
			// Cast<IClientUIInterface>(ClientUIComponent)->InitializeUI();
			RegistClientComponent(ClientUIComponent);
			
			UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::BeginPlay - Component initialization completed"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController::BeginPlay - ClientAuthComponentClass or ClientUIComponentClass is null"));
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

void AGGwaPlayerController::Request_Client_ConnectToGameServerWithToken(const FString& Token, const FString& UserId) {
	ConnectToGameServerWithToken(Token, UserId);
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

void AGGwaPlayerController::RegistClientComponent(UActorComponent* Component) {
	if (CachedClientManagerInterface.GetInterface())
	{
		CachedClientManagerInterface->RegistClientComponent(Component);
	}
}

void AGGwaPlayerController::InitializeUI(const USkillComponent* SkillComponent) {
	if (!IsLocalPlayerController()) return;
	
	// Additional validation for TScriptInterface
	if (CachedClientManagerInterface.GetInterface() && CachedClientManagerInterface.GetObject())
	{
		CachedClientManagerInterface->InitializeUI(SkillComponent);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::InitializeUI - Delegated to ClientManager"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::InitializeUI - ClientManagerInterface not available"));
		UE_LOG(LogTemp, Warning, TEXT("  - Interface: %s, Object: %s"), 
			CachedClientManagerInterface.GetInterface() ? TEXT("Valid") : TEXT("Null"),
			CachedClientManagerInterface.GetObject() ? TEXT("Valid") : TEXT("Null"));
			
		// Try to re-acquire the interface
		if (ClientAuthComponent)
		{
			if (auto AuthInterface = Cast<IClientAuthInterface>(ClientAuthComponent))
			{
				TScriptInterface<IClientManagerInterface> SubSystem = AuthInterface->GetClientSubSystem();
				if (SubSystem.GetInterface())
				{
					CachedClientManagerInterface = SubSystem;
					UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::InitializeUI - Re-acquired ClientManagerInterface, retrying..."));
					CachedClientManagerInterface->InitializeUI(SkillComponent);
				}
			}
		}
	}
}

void AGGwaPlayerController::ProcessRegistration(const FString& Username, const FString& Password) {
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
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
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
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
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
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
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
	{
		CachedClientManagerInterface->HandleLoginResult(bSuccess, Token, UserId);
		UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::HandleLoginResult - Success: %s, UserId: %s"), 
			bSuccess ? TEXT("true") : TEXT("false"), *UserId);
		
		// Token-based server connection is now handled by AuthSubsystem in OnGameDataLoaded
		// No need to call ConnectToGameServerWithToken here to avoid duplication
		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::HandleLoginResult - Login successful, AuthSubsystem will handle server connection"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::HandleLoginResult - ClientManagerInterface not available"));
	}
}

void AGGwaPlayerController::ProcessMouseOverDetection() {
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
	{
		CachedClientManagerInterface->ProcessMouseOverDetection();
		// No log here as this is called frequently
	}
	// No warning log for mouse detection as it's called frequently
}

void AGGwaPlayerController::NotifyStateChanged() {
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
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
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
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
	if (!IsLocalPlayerController()) return;
	
	if (CachedClientManagerInterface.GetInterface())
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
	if (!IsLocalPlayerController()) return;
	Super::PlayerTick(DeltaTime);
	ProcessMouseOverDetection();
}

// ============================================================================
// TOKEN-BASED SERVER CONNECTION
// ============================================================================

void AGGwaPlayerController::ConnectToGameServerWithToken(const FString& Token, const FString& UserId)
{
	if (!IsLocalPlayerController()) return;

	// Store token for server validation
	CachedAuthToken = Token;
	CachedUserId = UserId;
	
	// Detect PIE environment
	UWorld* World = GetWorld();
	bool bIsPIEEnvironment = World && World->WorldType == EWorldType::PIE;
	ENetMode NetMode = World ? World->GetNetMode() : NM_Standalone;
	
	UE_LOG(LogTemp, Warning, TEXT("=== GGwaPlayerController::ConnectToGameServerWithToken DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("Controller: %p | PIE: %s | NetMode: %d"), 
		this, bIsPIEEnvironment ? TEXT("Yes") : TEXT("No"), (int32)NetMode);
	UE_LOG(LogTemp, Warning, TEXT("Token: %s... | UserId: %s"), *Token.Left(20), *UserId);
	//
	// if (bIsPIEEnvironment)
	// {
	// 	// PIE 환경: LoginLevel에서 바로 서버로 연결 (PreLogin/PostLogin 테스트)
	// 	const FString PIEServerURL = FString::Printf(
	// 		TEXT("127.0.0.1:7777?token=%s&userid=%s&pie=true"),
	// 		*Token,
	// 		*UserId
	// 	);
	// 	
	// 	UE_LOG(LogTemp, Warning, TEXT("PIE Environment: Connecting with token authentication - %s"), *PIEServerURL);
	// 	
	// 	// PIE에서 TRAVEL_Absolute 사용하여 실제 서버 연결 흐름 테스트
	// 	// 이는 PreLogin/PostLogin을 정상적으로 호출함
	// 	ClientTravel(PIEServerURL, TRAVEL_Absolute);
	// 	return;
	// }
	
	// Production environment: Connect to actual server
	const FString ServerIP = TEXT("127.0.0.1");
	const FString ServerPort = TEXT("7777");
	const FString ServerURL = FString::Printf(
		TEXT("%s:%s?token=%s&userid=%s"),
		*ServerIP,
		*ServerPort,
		*Token,
		*UserId
	);
	
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::ConnectToGameServerWithToken - Connecting to: %s"), *ServerURL);
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::ConnectToGameServerWithToken - Token: %s"), *Token.Left(20)); // Log first 20 chars for security

	// Production: TRAVEL_Absolute for inter-server connections
	// Note: This will create a new PlayerController instance, losing cached components
	// Component re-initialization will be handled in BeginPlay()
	ClientTravel(ServerURL, TRAVEL_Absolute);
}
