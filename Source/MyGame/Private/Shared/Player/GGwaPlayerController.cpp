// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "AuthModule/Public/AuthSubsystem.h"

// Client-only includes
#if !UE_SERVER
#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "MyGame/Public/Shared/AI/EnemyAbilitySystemComponent.h"
#include "MyGame/Public/Shared/AI/EnemyAttributeSet.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"

// These headers are from ClientModule - only include if available
#ifdef CLIENTMODULE_API
#include "ClientModule/Public/UI/UIManagerSubsystem.h"
#include "ClientModule/Public/UI/Widget/GGwaWidget.h"
#include "ClientModule/Public/UI/GGwaHUD.h"
#include "ClientModule/Public/UI/Enemy/BossStatusWidget.h"
#include "AuthClientModule/Public/AuthService.h"
#endif
#endif

AGGwaPlayerController::AGGwaPlayerController() {
}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	
	// Client-only UI setup
	if (IsLocalController()) {
		bShowMouseCursor = true;
		FInputModeGameAndUI inputMode;
		inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		inputMode.SetHideCursorDuringCapture(false);
		SetInputMode(inputMode);

#if !UE_SERVER
		bEnableMouseOverEvents = true;

#ifdef CLIENTMODULE_API
		// Create AuthService instance for client
		AuthService = NewObject<UAuthService>(this, TEXT("AuthService"));
		if (!AuthService)
		{
			UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController: Failed to create AuthService!"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController: AuthService created successfully"));
		}
#endif
#endif
	}
	
	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
}


// 해당 아래 RPC들이 OnReplicatedUsing에 비해 안정적인지 검토해봐야한다.
// void AGGwaPlayerController::Client_ApplyAbilityDataAsset_Implementation(UBaseDataAsset* Data) {
// 	UE_LOG(LogTemp, Log, TEXT("ApplyAbility Data Called From Server"));
// }

void AGGwaPlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& BossCharacter) {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController()) {
		OnBossDataReceived.Broadcast(BossCharacter);
	}
#endif
	UE_LOG(LogTemp, Log, TEXT("BossData Called From Server"));
}

void AGGwaPlayerController::Client_ReceiveSkillData_Implementation(const USkillComponent* SkillComponent) {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController() && GGwaHUD && GGwaHUD->GetBaseWidget()) {
		GGwaHUD->GetBaseWidget()->UpdateSkillWidgetFromServer(SkillComponent);
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

#if !UE_SERVER && defined(CLIENTMODULE_API)
	// Notify AuthService about the result
	if (AuthService)
	{
		AuthService->OnServerRegistrationResult(bSuccess, Message);
	}
	
	// Also forward to Blueprint for UI updates
	OnRegistrationResult_BP(bSuccess, Message);
#endif
}

void AGGwaPlayerController::Client_OnLoginResult_Implementation(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_OnLoginResult: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

#if !UE_SERVER && defined(CLIENTMODULE_API)
	// Notify AuthService about the result
	if (AuthService)
	{
		AuthService->OnServerLoginResult(bSuccess, Token, UserId);
	}

	// Also forward to Blueprint for UI updates
	OnLoginResult_BP(bSuccess, Token, UserId);

	// If login successful, we might need to wait for Client_TravelToGameWorld call
	// or handle any UI updates here
#endif
}

void AGGwaPlayerController::Client_TravelToGameWorld_Implementation(const FString& MapURL)
{
	UE_LOG(LogTemp, Log, TEXT("AGGwaPlayerController::Client_TravelToGameWorld: Traveling to %s"), *MapURL);
	
#if !UE_SERVER && defined(CLIENTMODULE_API)
	// Perform client travel to the game world
	ClientTravel(MapURL, TRAVEL_Relative);
	GetGameInstance()->GetSubsystem<UUIManagerSubsystem>()->OnMapChanged(GetWorld());
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
#if !UE_SERVER

void AGGwaPlayerController::InitClientWidget(const USkillComponent* SkillComponent) {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (HasAuthority()) {
		UE_LOG(LogTemp, Warning, TEXT("server Controller"));
		return;
	}
	else if (IsLocalController()) {
		UE_LOG(LogTemp, Warning, TEXT("Client has been initialized"));
	}
	
	if (WidgetClass) {
		UGGwaWidget* Widget = CreateWidget<UGGwaWidget>(this, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(this, BossStatusWidgetClass);
		Widget->AddToViewport();
		BossWidget->AddToViewport();
		BossWidget->SetVisibility(ESlateVisibility::Hidden);
		
		if (Widget) {
			GGwaHUD = Cast<AGGwaHUD>(GetHUD());
			GGwaHUD->SetBaseWidget(Widget);
			GGwaHUD->SetBossWidget(BossWidget);

			// Bind to the controller's delegates
			OnBossDataReceived.AddDynamic(GGwaHUD, &AGGwaHUD::HandleBossDataReceived);
			
			if (AGGwaPlayerState* PS = GetPlayerState<AGGwaPlayerState>()) {
				auto ASC = PS->GetAbilitySystemComponent();
				UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
				const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				Widget->UpdateSkillWidgetFromServer(SkillComponent);
				Widget->InitWidget(GGawASC, GGwaAttributeSet);
			}
		}
	}
#else
	// Server does not need UI initialization
#endif
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// Client-only mouse over logic
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController()) {
		FHitResult Hit;
		// Visibility 채널로 마우스 밑 Actor 판별
		if (GetHitResultUnderCursorByChannel(
				UEngineTypes::ConvertToTraceType(ECC_Visibility),
				true, Hit))
		{
			if (ABossCharacter* Enemy = Cast<ABossCharacter>(Hit.GetActor()))
			{
				if (GGwaHUD) {
					GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Visible);
				}
				
				if (Enemy != LastHoveredEnemy.Get()) {
					LastHoveredEnemy = Enemy;

					// ASC와 AttributeSet 가져오기
					UEnemyAbilitySystemComponent* ASC = Cast<UEnemyAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
					const UEnemyAttributeSet* AttrSet = ASC ? ASC->GetSet<UEnemyAttributeSet>() : nullptr;
					if (!ASC || !AttrSet) return;

					FBossDataStruct BossData;
					BossData.Health = AttrSet->GetHealth();
					BossData.MaxHealth = AttrSet->GetMaxHealth();
					BossData.Damage = AttrSet->GetDamage();

					// FEnemyWidgetData 구성
					FEnemyWidgetData WidgetData = Enemy->GetWidgetData();
					if (GGwaHUD && IsLocalController()) {
						GGwaHUD->GetBossWidget()->SetWidget(WidgetData, BossData);
					}
				}
				return;
			}
		}

		// 커서가 적 이외 영역에 있을 때: 클리어
		if (LastHoveredEnemy.IsValid()) {
			LastHoveredEnemy = nullptr;
			if (GGwaHUD && IsLocalController()) {
				GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
#endif
}

void AGGwaPlayerController::NotifyClientStateChanged() {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController() && GGwaHUD && GGwaHUD->GetBaseWidget()) {
		GGwaHUD->GetBaseWidget()->OnPlayerStateChanged.Broadcast();
	}
#endif
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
#if !UE_SERVER && defined(CLIENTMODULE_API)
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::RequestRegistration: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController: AuthService not available"));
		OnRegistrationResult_BP(false, TEXT("Authentication service not available"));
		return;
	}

	// Create delegate for registration result
	FRegistrationDelegate RegistrationDelegate;
	RegistrationDelegate.BindDynamic(this, &AGGwaPlayerController::OnRegistrationComplete);

	// Request registration through AuthService
	AuthService->RequestRegistration(Username, Password, RegistrationDelegate);
#else
	UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::RequestRegistration: Called on server or without ClientModule"));
#endif
}

void AGGwaPlayerController::RequestLogin(const FString& Username, const FString& Password)
{
#if !UE_SERVER && defined(CLIENTMODULE_API)
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::RequestLogin: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("GGwaPlayerController: AuthService not available"));
		OnLoginResult_BP(false, TEXT(""), TEXT(""));
		return;
	}

	// Create delegate for login result
	FLoginDelegate LoginDelegate;
	LoginDelegate.BindDynamic(this, &AGGwaPlayerController::OnLoginComplete);

	// Request login through AuthService
	AuthService->RequestLogin(Username, Password, LoginDelegate);
#else
	UE_LOG(LogTemp, Warning, TEXT("GGwaPlayerController::RequestLogin: Called on server or without ClientModule"));
#endif
}

// ============================================================================
// AuthService Callbacks (Client-side)
// ============================================================================

void AGGwaPlayerController::OnRegistrationComplete(bool bSuccess, const FString& Message)
{
#if !UE_SERVER && defined(CLIENTMODULE_API)
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::OnRegistrationComplete: Success=%s, Message=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *Message);

	// Forward to Blueprint
	OnRegistrationResult_BP(bSuccess, Message);
#endif
}

void AGGwaPlayerController::OnLoginComplete(bool bSuccess, const FString& Token, const FString& UserId)
{
#if !UE_SERVER && defined(CLIENTMODULE_API)
	UE_LOG(LogTemp, Log, TEXT("GGwaPlayerController::OnLoginComplete: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

	// Forward to Blueprint
	OnLoginResult_BP(bSuccess, Token, UserId);

	// If login successful, could automatically connect to game server
	// For now, we let Blueprint handle the flow
#endif
}
#endif // !UE_SERVER

