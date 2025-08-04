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

void UClientAuthComponent::InitializeAuth()
{
	OwnerController = Cast<AGGwaPlayerController>(GetOwner());
	if (!OwnerController  || !OwnerController->IsLocalPlayerController()){
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Owner is not AGGwaPlayerController"));
		return;
	}

	AuthService = NewObject<UAuthService>(this, TEXT("AuthService"));
	if (!AuthService){
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: Failed to create AuthService!"));
	}
}

void UClientAuthComponent::RequestRegistration(const FString& Username, const FString& Password)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::RequestRegistration: Username=%s"), *Username);

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientAuthComponent: AuthService not available"));
		return;
	}

	AuthService->RequestRegistration(Username, Password, OwnerController);
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

	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent: Successfully bound login delegate"));

	AuthService->RequestLogin(Username, Password, OwnerController);
}

void UClientAuthComponent::OnServerRegistrationResult(bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::OnServerRegistrationResult: Success=%s, Message=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *Message);
	
	BP_OnRegistrationResult(bSuccess, Message);
}

void UClientAuthComponent::OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("ClientAuthComponent::OnServerLoginResult: Success=%s, UserId=%s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *UserId);

	BP_OnLoginResult(bSuccess, Token, UserId);
}

TScriptInterface<IClientManagerInterface> UClientAuthComponent::GetClientSubSystem() {
	if (auto UISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>()) {
		return UISubsystem;
	}
	return nullptr;
}
