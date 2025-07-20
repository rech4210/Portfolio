#include "Player/ClientAuthComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "AuthClientModule/Public/AuthService.h"
#include "Engine/Engine.h"
#include "UI/UIManagerSubsystem.h"

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
		InitializeAuth();
		
		// Register this component as AuthService implementation using helper method
		RegisterSelfToServiceManager();
		
		UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent: Initialized and registered for local controller"));
	}
}

// ============================================================================
// IClientAuthInterface IMPLEMENTATION
// ============================================================================

void UClientAuthComponent::InitializeAuth()
{
	if (!OwnerController || !OwnerController->IsLocalController())
	{
		return;
	}

	// Create AuthService instance
	AuthService = NewObject<UAuthService>(this, TEXT("AuthService"));
	if (AuthService)
	{
		UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent: AuthService created successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Failed to create AuthService!"));
	}
}

void UClientAuthComponent::RequestRegistration(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::RequestRegistration: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: AuthService not available"));
		BP_OnRegistrationResult(false, TEXT("Authentication service not available"));
		return;
	}

	// Create delegate for registration result
	FRegistrationDelegate RegistrationDelegate;
	RegistrationDelegate.BindDynamic(this, &UClientAuthComponent::OnServerRegistrationResult);

	// Request registration through AuthService
	AuthService->RequestRegistration(Username, Password, OwnerController, RegistrationDelegate);
}

void UClientAuthComponent::RequestLogin(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::RequestLogin: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: AuthService not available"));
		BP_OnLoginResult(false, TEXT(""), TEXT(""));
		return;
	}

	// Create delegate for login result
	FLoginDelegate LoginDelegate;
	LoginDelegate.BindDynamic(this, &UClientAuthComponent::OnServerLoginResult);

	// Request login through AuthService
	AuthService->RequestLogin(Username, Password, OwnerController, LoginDelegate);
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
	BP_OnRegistrationResult(bSuccess, Message);
	
	// Also forward to PlayerController's Blueprint event for backward compatibility
	// if (OwnerController)
	// {
	// 	OwnerController->OnRegistrationResult_BP(bSuccess, Message);
	// }
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
	BP_OnLoginResult(bSuccess, Token, UserId);
	
	// Also forward to PlayerController's Blueprint event for backward compatibility
	// if (OwnerController)
	// {
	// 	OwnerController->OnLoginResult_BP(bSuccess, Token, UserId);
	// }
}

// ============================================================================
// PRIVATE HELPER METHODS
// ============================================================================

void UClientAuthComponent::RegisterSelfToServiceManager()
{
	if (OwnerController)
	{
		// Create TScriptInterface for this component
		TScriptInterface<IClientAuthInterface> AuthInterface;
		AuthInterface.SetObject(this);
		AuthInterface.SetInterface(static_cast<IClientAuthInterface*>(this));

		// Register with UIManagerSubsystem
		if (UWorld* World = GetWorld())
		{
			if (UUIManagerSubsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>())
			{
				UISubsystem->RegisterAuthService(AuthInterface);
				UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent: Successfully registered to UIManagerSubsystem"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ClientAuthComponent: Failed to get UIManagerSubsystem"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Cannot register - OwnerController is null"));
	}
}
