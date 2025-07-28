
// #include "Player/LoginPlayerController.h"
// #include "Blueprint/UserWidget.h"
// #include "Engine/Engine.h"
//
// void ALoginPlayerController::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	// Create AuthService instance
// 	AuthService = NewObject<UAuthService>(this, TEXT("AuthService"));
//
// 	if (!AuthService)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("LoginPlayerController: Failed to create AuthService!"));
// 		return;
// 	}
//
// 	// Create login UI for local controller
// 	if (IsLocalController() && LoginWidgetClass)
// 	{
// 		LoginWidgetInstance = CreateWidget<UUserWidget>(this, LoginWidgetClass);
// 		if (LoginWidgetInstance)
// 		{
// 			LoginWidgetInstance->AddToViewport();
//             
// 			// Setup UI interaction
// 			bShowMouseCursor = true;
// 			SetInputMode(FInputModeUIOnly());
//
// 			UE_LOG(LogTemp, Log, TEXT("LoginPlayerController: Login UI created successfully"));
// 		}
// 	}
// }
//
// void ALoginPlayerController::RequestRegistration(const FString& Username, const FString& Password)
// {
// 	UE_LOG(LogTemp, Log, TEXT("LoginPlayerController::RequestRegistration: Username=%s"), *Username);
//
// 	if (!AuthService)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("LoginPlayerController: AuthService not available"));
// 		OnRegistrationResult_BP(false, TEXT("Authentication service not available"));
// 		return;
// 	}
//
// 	// Create delegate for registration result
// 	FRegistrationDelegate RegistrationDelegate;
// 	RegistrationDelegate.BindDynamic(this, &ALoginPlayerController::OnRegistrationComplete);
//
// 	// Request registration through AuthService
// 	AuthService->RequestRegistration(Username, Password, RegistrationDelegate);
// }
//
// void ALoginPlayerController::RequestLogin(const FString& Username, const FString& Password)
// {
// 	UE_LOG(LogTemp, Log, TEXT("LoginPlayerController::RequestLogin: Username=%s"), *Username);
//
// 	if (!AuthService)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("LoginPlayerController: AuthService not available"));
// 		OnLoginResult_BP(false, TEXT(""), TEXT(""));
// 		return;
// 	}
//
// 	// Create delegate for login result
// 	FLoginDelegate LoginDelegate;
// 	LoginDelegate.BindDynamic(this, &ALoginPlayerController::OnLoginComplete);
//
// 	// Request login through AuthService
// 	AuthService->RequestLogin(Username, Password, LoginDelegate);
// }
//
// void ALoginPlayerController::ConnectToGameServer(const FString& ServerAddress, const FString& Token)
// {
// 	UE_LOG(LogTemp, Log, TEXT("LoginPlayerController::ConnectToGameServer: Connecting to %s"), *ServerAddress);
//
// 	// Hide login UI
// 	if (LoginWidgetInstance)
// 	{
// 		LoginWidgetInstance->RemoveFromParent();
// 		LoginWidgetInstance = nullptr;
// 	}
//
// 	// Set input mode back to game
// 	bShowMouseCursor = false;
// 	SetInputMode(FInputModeGameOnly());
//
// 	// Format connection URL with token
// 	FString ConnectURL;
// 	if (Token.IsEmpty())
// 	{
// 		ConnectURL = ServerAddress;
// 	}
// 	else
// 	{
// 		// Add token as query parameter if provided
// 		ConnectURL = FString::Printf(TEXT("%s?token=%s"), *ServerAddress, *Token);
// 	}
//
// 	// Travel to game server
// 	ClientTravel(ConnectURL, TRAVEL_Absolute);
// }
//
// // ============================================================================
// // AuthService Callbacks
// // ============================================================================
//
// void ALoginPlayerController::OnRegistrationComplete(bool bSuccess, const FString& Message)
// {
// 	UE_LOG(LogTemp, Log, TEXT("LoginPlayerController::OnRegistrationComplete: Success=%s, Message=%s"), 
// 		bSuccess ? TEXT("true") : TEXT("false"), *Message);
//
// 	// Forward to Blueprint
// 	OnRegistrationResult_BP(bSuccess, Message);
// }
//
// void ALoginPlayerController::OnLoginComplete(bool bSuccess, const FString& Token, const FString& UserId)
// {
// 	UE_LOG(LogTemp, Log, TEXT("LoginPlayerController::OnLoginComplete: Success=%s, UserId=%s"), 
// 		bSuccess ? TEXT("true") : TEXT("false"), *UserId);
//
// 	// Forward to Blueprint
// 	OnLoginResult_BP(bSuccess, Token, UserId);
//
// 	// If login successful, could automatically connect to game server
// 	// For now, we let Blueprint handle the flow
// }
