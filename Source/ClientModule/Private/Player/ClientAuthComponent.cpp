#include "Player/ClientAuthComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "AuthClientModule/Public/AuthService.h"
#include "Engine/Engine.h"

UClientAuthComponent::UClientAuthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	AuthService = nullptr;
	OwnerController = nullptr;
}

void UClientAuthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache owner controller reference
	OwnerController = Cast<AGGwaPlayerController>(GetOwner());
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Owner is not AGGwaPlayerController"));
		return;
	}

	// Only initialize on client and if locally controlled
	if (OwnerController->IsLocalController())
	{
		InitializeAuthService();
		UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent: Initialized for local controller"));
	}
}

// ============================================================================
// CLIENT AUTHENTICATION INTERFACE
// ============================================================================

void UClientAuthComponent::InitializeAuthService()
{
	if (!OwnerController || !OwnerController->IsLocalController())
	{
		return;
	}

	// Create AuthService instance
	AuthService = NewObject<UAuthService>(this, TEXT("AuthService"));
	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Failed to create AuthService!"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent: AuthService created successfully"));
	}
}

void UClientAuthComponent::RequestRegistration(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::RequestRegistration: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: AuthService not available"));
		OnRegistrationResult_BP(false, TEXT("Authentication service not available"));
		return;
	}

	// Create delegate for registration result
	FRegistrationDelegate RegistrationDelegate;
	RegistrationDelegate.BindDynamic(this, &UClientAuthComponent::OnRegistrationComplete);

	// Request registration through AuthService
	AuthService->RequestRegistration(Username, Password, RegistrationDelegate);
}

void UClientAuthComponent::RequestLogin(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::RequestLogin: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: AuthService not available"));
		OnLoginResult_BP(false, TEXT(""), TEXT(""));
		return;
	}

	// Create delegate for login result
	FLoginDelegate LoginDelegate;
	LoginDelegate.BindDynamic(this, &UClientAuthComponent::OnLoginComplete);

	// Request login through AuthService
	AuthService->RequestLogin(Username, Password, LoginDelegate);
}

void UClientAuthComponent::OnServerRegistrationResult(bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::OnServerRegistrationResult: Success=%s, Message=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *Message);

	// Notify AuthService about the result
	if (AuthService)
	{
		AuthService->OnServerRegistrationResult(bSuccess, Message);
	}
	
	// Forward to Blueprint for UI updates
	OnRegistrationResult_BP(bSuccess, Message);
	
	// Also forward to PlayerController's Blueprint event for backward compatibility
	if (OwnerController)
	{
		OwnerController->OnRegistrationResult_BP(bSuccess, Message);
	}
}

void UClientAuthComponent::OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::OnServerLoginResult: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

	// Notify AuthService about the result
	if (AuthService)
	{
		AuthService->OnServerLoginResult(bSuccess, Token, UserId);
	}

	// Forward to Blueprint for UI updates
	OnLoginResult_BP(bSuccess, Token, UserId);
	
	// Also forward to PlayerController's Blueprint event for backward compatibility
	if (OwnerController)
	{
		OwnerController->OnLoginResult_BP(bSuccess, Token, UserId);
	}
}

// ============================================================================
// AUTHSERVICE CALLBACK HANDLERS
// ============================================================================

void UClientAuthComponent::OnRegistrationComplete(bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::OnRegistrationComplete: Success=%s, Message=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *Message);

	// Forward to Blueprint
	OnRegistrationResult_BP(bSuccess, Message);
	
	// Also forward to PlayerController's Blueprint event if needed
	if (OwnerController)
	{
		OwnerController->OnRegistrationResult_BP(bSuccess, Message);
	}
}

void UClientAuthComponent::OnLoginComplete(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::OnLoginComplete: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

	// Forward to Blueprint
	OnLoginResult_BP(bSuccess, Token, UserId);
	
	// Also forward to PlayerController's Blueprint event if needed
	if (OwnerController)
	{
		OwnerController->OnLoginResult_BP(bSuccess, Token, UserId);
	}
}
