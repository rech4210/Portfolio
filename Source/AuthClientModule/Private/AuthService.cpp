#include "AuthService.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameSharedModule/Public/Interface/AuthRPCInterface.h"

UAuthService::UAuthService()
{
}

void UAuthService::RequestRegistration(const FString& Username, const FString& Password, APlayerController* PC)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Requesting registration for user %s"), *Username);

    if (Username.IsEmpty() || Password.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthService: Invalid registration parameters"));

        return;
    }

    if (IAuthRPCInterface* AuthRPC = Cast<IAuthRPCInterface>(PC))
    {
        if (AuthRPC->IsAuthRPCAvailable())
        {

            AuthRPC->RequestServerRegistration(Username, Password);
            
            UE_LOG(LogTemp, Log, TEXT("AuthService: Registration request sent through AuthRPC interface"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AuthService: AuthRPC interface not available"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AuthService: PlayerController does not implement AuthRPC interface"));

    }
}

void UAuthService::RequestLogin(const FString& Username, const FString& Password, APlayerController* PC)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Requesting login for user %s"), *Username);

    if (Username.IsEmpty() || Password.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthService: Invalid login parameters"));

        return;
    }
    if (IAuthRPCInterface* AuthRPC = Cast<IAuthRPCInterface>(PC))
    {
        if (AuthRPC->IsAuthRPCAvailable())
        {
            AuthRPC->RequestServerLogin(Username, Password);
            
            UE_LOG(LogTemp, Log, TEXT("AuthService: Login request sent through AuthRPC interface"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AuthService: AuthRPC interface not available"));

        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AuthService: PlayerController does not implement AuthRPC interface"));

    }
}

void UAuthService::Logout()
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Logging out user %s"), *CurrentUserId);
    
    CurrentToken.Empty();
    CurrentUserId.Empty();
    bIsAuthenticated = false;
}

bool UAuthService::IsAuthenticated() const
{
    return bIsAuthenticated && !CurrentToken.IsEmpty() && !CurrentUserId.IsEmpty();
}

FString UAuthService::GetCurrentToken() const
{
    return CurrentToken;
}

FString UAuthService::GetCurrentUserId() const
{
    return CurrentUserId;
}

