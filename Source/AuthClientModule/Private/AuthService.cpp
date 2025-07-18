#include "AuthService.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameSharedModule/Public/Interface/AuthRPCInterface.h"

UAuthService::UAuthService()
{
    // No longer needs direct server communication
    // Will communicate through AuthRPC interface
}

void UAuthService::RequestRegistration(const FString& Username, const FString& Password, APlayerController* PC , FRegistrationDelegate OnResult)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Requesting registration for user %s"), *Username);

    if (Username.IsEmpty() || Password.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthService: Invalid registration parameters"));
        OnResult.ExecuteIfBound(false, TEXT("Username and password are required"));
        return;
    }

    // Get PlayerController and try to use AuthRPC interface
    if (UWorld* World = GetWorld())
    {

        // Try to cast to AuthRPC interface
        if (IAuthRPCInterface* AuthRPC = Cast<IAuthRPCInterface>(PC))
        {
            if (AuthRPC->IsAuthRPCAvailable())
            {
                // Store delegate for later callback
                PendingRegistrationDelegate = OnResult;
                
                // Call interface method for registration
                AuthRPC->RequestServerRegistration(Username, Password);
                
                UE_LOG(LogTemp, Log, TEXT("AuthService: Registration request sent through AuthRPC interface"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AuthService: AuthRPC interface not available"));
                OnResult.ExecuteIfBound(false, TEXT("Authentication service unavailable"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AuthService: PlayerController does not implement AuthRPC interface"));
            OnResult.ExecuteIfBound(false, TEXT("Authentication interface not supported"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AuthService: No World context"));
        OnResult.ExecuteIfBound(false, TEXT("Invalid game state"));
    }
}

void UAuthService::RequestLogin(const FString& Username, const FString& Password, APlayerController* PC, FLoginDelegate OnResult)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Requesting login for user %s"), *Username);

    if (Username.IsEmpty() || Password.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthService: Invalid login parameters"));
        OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Username and password are required"));
        return;
    }

    // Get PlayerController and try to use AuthRPC interface
    if (UWorld* World = GetWorld())
    {

        if (IAuthRPCInterface* AuthRPC = Cast<IAuthRPCInterface>(PC))
        {
            if (AuthRPC->IsAuthRPCAvailable())
            {
                // Store delegate for later callback
                PendingLoginDelegate = OnResult;
                
                // Call interface method for login
                AuthRPC->RequestServerLogin(Username, Password);
                
                UE_LOG(LogTemp, Log, TEXT("AuthService: Login request sent through AuthRPC interface"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AuthService: AuthRPC interface not available"));
                OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Authentication service unavailable"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AuthService: PlayerController does not implement AuthRPC interface"));
            OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Authentication interface not supported"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AuthService: No World context"));
        OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Invalid game state"));
    }
}

void UAuthService::Logout()
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Logging out user %s"), *CurrentUserId);
    
    CurrentToken.Empty();
    CurrentUserId.Empty();
    bIsAuthenticated = false;
    
    // TODO: Notify server of logout if needed
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



// ============================================================================
// AUTHSERVICE CALLBACK HANDLERS
// ============================================================================

void UAuthService::OnServerRegistrationResult(bool bSuccess, const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Server registration result - Success: %s, Message: %s"), 
        bSuccess ? TEXT("true") : TEXT("false"), *Message);

    if (PendingRegistrationDelegate.IsBound())
    {
        PendingRegistrationDelegate.ExecuteIfBound(bSuccess, Message);
        PendingRegistrationDelegate.Unbind();
    }
}

void UAuthService::OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Server login result - Success: %s, UserId: %s"), 
        bSuccess ? TEXT("true") : TEXT("false"), *UserId);

    if (PendingLoginDelegate.IsBound())
    {
        if (bSuccess)
        {
            // Store authentication state
            CurrentToken = Token;
            CurrentUserId = UserId;
            bIsAuthenticated = true;
            
            PendingLoginDelegate.ExecuteIfBound(true, Token, UserId);
        }
        else
        {
            bIsAuthenticated = false;
            PendingLoginDelegate.ExecuteIfBound(false, TEXT(""), TEXT("Authentication failed"));
        }
        PendingLoginDelegate.Unbind();
    }
}
