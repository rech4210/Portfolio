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

}

// ============================================================================
// IClientAuthInterface IMPLEMENTATION
// ============================================================================

void UClientAuthComponent::InitializeAuth()
{
	OwnerController = Cast<AGGwaPlayerController>(GetOwner());
	if (!OwnerController){
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Owner is not AGGwaPlayerController"));
		return;
	}
	
	if (!OwnerController || !OwnerController->IsLocalController()){
		return;
	}

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

IClientManagerInterface* UClientAuthComponent::GetClientSubSystem() {
	if (auto UISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>()) {
		return UISubsystem;
	}
	return nullptr;
}

// ============================================================================
// PRIVATE HELPER METHODS
// ============================================================================
