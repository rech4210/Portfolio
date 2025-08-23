#include "Shared/Player/GGwaPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "AuthModule/Public/AuthSubsystem.h"
#include "Engine/World.h"
#include "GameSharedModule/Public/Utils/ClientUIMapping.h"

AGGwaPlayerController::AGGwaPlayerController() {
}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	TryBindASCInput();
#if !UE_SERVER
	InitializeClientComponent();
	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
#endif
}


void AGGwaPlayerController::TryBindASCInput() {
	const AGGwaPlayerState* PS = GetPlayerState<AGGwaPlayerState>();
	if (!IsValid(PS) || bIsBindComplete || !InputComponent) {
		return;
	}
	
	UGGwaAbilitySystemComponent * ASC = Cast<UGGwaAbilitySystemComponent>( GetPlayerState<AGGwaPlayerState>()->GetAbilitySystemComponent());
	if (!InputComponent || !ASC) {
		return;
	}
	
	if (ASC) {
		ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds("Confirm", "Cancel", FTopLevelAssetPath(TEXT("/Script/SkillModule"), TEXT("EAbilityInputID"))));
		bIsBindComplete = true;
	}
}

void AGGwaPlayerController::AcknowledgePossession(class APawn* PossessedPawn) {
	Super::AcknowledgePossession(PossessedPawn);
	TryBindASCInput();
}

void AGGwaPlayerController::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	if (GetPlayerState<AGGwaPlayerState>()) {
		InitializeUI();
	}
}


void AGGwaPlayerController::Server_InitiateReward_Implementation(const FString& PlayerId, const FRewardRequest& Payload) {
	IServerLogicBridge* Bridge = Cast<IServerLogicBridge>(GetWorld()->GetSubsystem<UWorldSubsystem>());
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
		AuthSubsystem->RequestServerRegistration(Username, Password, ClientIP, this);
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

	FString ClientIP = TEXT("Unknown");
	if (GetNetConnection() && GetNetConnection()->RemoteAddr.IsValid())
	{
		ClientIP = GetNetConnection()->RemoteAddr->ToString(false);
	}

	if (UAuthSubsystem* AuthSubsystem = GetGameInstance()->GetSubsystem<UAuthSubsystem>())
	{
		AuthSubsystem->RequestServerAuthentication(Username, Password, ClientIP, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaPlayerController::Server_Login: AuthSubsystem not found"));
	}
}

bool AGGwaPlayerController::Server_Login_Validate(const FString& Username, const FString& Password)
{
	return !Username.IsEmpty() && !Password.IsEmpty() && Username.Len() <= 30 && Password.Len() <= 128;
}

void AGGwaPlayerController::Client_TravelToGameWorld_Implementation(const FString& MapURL)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_TravelToGameWorld: Traveling to %s"), *MapURL);
	
#if !UE_SERVER
	ClientTravel(MapURL, TRAVEL_Relative);
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

void AGGwaPlayerController::InitializeUI() {
	if (!IsLocalPlayerController()) return;
#if !UE_SERVER
	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] InitializeUI called on client"));
	
	if (!CachedClientManagerInterface) {
		InitializeClientComponent();
	}
	
	if (ClientAuthComponent)
	{
		if (auto AuthInterface = Cast<IClientAuthInterface>(ClientAuthComponent))
		{
			TScriptInterface<IClientManagerInterface> SubSystem = AuthInterface->GetClientSubSystem();
			if (SubSystem.GetInterface())
			{
				CachedClientManagerInterface = SubSystem;
				UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::InitializeUI - Re-acquired ClientManagerInterface, retrying..."));
				CachedClientManagerInterface->InitializeUI();
			}
		}
	}
#endif
}

void AGGwaPlayerController::InitializeClientComponent() {
	UE_LOG(LogTemp, Warning, TEXT("InitializeClientComponent : Called"));

#if !UE_SERVER
	if (IsLocalPlayerController())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("- Client Only Controller::BeginPlay - World is null!"));
			return;
		}
		
		CreateDefaultClientComponents();
	}
#endif
}


void AGGwaPlayerController::CreateDefaultClientComponents()
{
#if !UE_SERVER
	ClientAuthComponent = CreateUIComponent(EClientUIKey::AuthComponent);
	if (ClientAuthComponent)
	{
		if (auto AuthInterface = Cast<IClientAuthInterface>(ClientAuthComponent))
		{
			AuthInterface->InitializeAuth();
		
			if (TScriptInterface<IClientManagerInterface> SubSystem = AuthInterface->GetClientSubSystem())
			{
				CachedClientManagerInterface = SubSystem;
				RegistClientComponent(ClientAuthComponent);
				UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::CreateDefaultClientComponents - CachedClientManagerInterface successfully assigned"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController::CreateDefaultClientComponents - Failed to get ClientSubSystem"));
			}
		}
	}
	
	ClientUIComponent = CreateUIComponent(EClientUIKey::LoginUI);
	if (ClientUIComponent)
	{
		if (auto UIInterface = Cast<IClientUIInterface>(ClientUIComponent))
		{
			RegistClientComponent(ClientUIComponent);
			UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::CreateDefaultClientComponents - UI component registered"));
		}
	}
#endif
}


UActorComponent* AGGwaPlayerController::CreateUIComponent(EClientUIKey UIKey)
{
#if !UE_SERVER
	UClass* ComponentClass = FClientUIMapping::LoadUIClass(UIKey);
	if (!ComponentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController::CreateUIComponent - Failed to load class for key: %d"), (int32)UIKey);
		return nullptr;
	}
	
	UActorComponent* NewComponent = NewObject<UActorComponent>(this, ComponentClass);
	if (!NewComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController::CreateUIComponent - Failed to create component instance"));
		return nullptr;
	}
	
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::CreateUIComponent - Created component for key: %d"), (int32)UIKey);
	return NewComponent;
#else
	UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::CreateUIComponent - Called on server, ignoring"));
	return nullptr;
#endif
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

	if (!CachedClientManagerInterface) {
		InitializeClientComponent();
	}
	CachedClientManagerInterface->ProcessBossData(BossData);
}

void AGGwaPlayerController::SkillHUDReplication(const FSkillSlotReplicationArray& SkillSlotsReplication) {
	if (!IsLocalPlayerController()) return;

	if (!CachedClientManagerInterface) {
		InitializeClientComponent();
	}
	CachedClientManagerInterface->SkillHUDReplication(SkillSlotsReplication);
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	if (!IsLocalPlayerController()) return;
	Super::PlayerTick(DeltaTime);
	ProcessMouseOverDetection();
}

void AGGwaPlayerController::ProcessMouseOverDetection() {
	
	if (CachedClientManagerInterface.GetInterface())
	{
		CachedClientManagerInterface->ProcessMouseOverDetection();
	}
}

// ============================================================================
// TOKEN-BASED SERVER CONNECTION
// ============================================================================

void AGGwaPlayerController::ConnectToGameServerWithToken(const FString& Token, const FString& UserId)
{
	if (!IsLocalPlayerController()) return;

	CachedAuthToken = Token;
	CachedUserId = UserId;
	
	UWorld* World = GetWorld();
	bool bIsPIEEnvironment = World && World->WorldType == EWorldType::PIE;
	ENetMode NetMode = World ? World->GetNetMode() : NM_Standalone;
	
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

	HandleLoginResult(true, Token, UserId);
	ClientTravel(ServerURL, TRAVEL_Relative);
}

