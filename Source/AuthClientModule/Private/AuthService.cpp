#include "AuthService.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

// Forward declaration - include the actual header if available
class AGGwaPlayerController;

UAuthService::UAuthService()
{
    // No longer needs direct server communication
    // Will communicate through PlayerController RPCs
}

void UAuthService::RequestRegistration(const FString& Username, const FString& Password, FRegistrationDelegate OnResult)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Requesting registration for user %s"), *Username);

    if (Username.IsEmpty() || Password.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthService: Invalid registration parameters"));
        OnResult.ExecuteIfBound(false, TEXT("Username and password are required"));
        return;
    }

    // Get PlayerController and call server RPC
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            // Cast to our custom PlayerController that has authentication RPCs
            if (AGGwaPlayerController* GGwaPC = Cast<AGGwaPlayerController>(PC))
            {
                // Store delegate for later callback
                PendingRegistrationDelegate = OnResult;
                
                // Call Server RPC for registration
                GGwaPC->Server_Register(Username, Password);
                
                UE_LOG(LogTemp, Log, TEXT("AuthService: Registration request sent to server"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AuthService: PlayerController is not AGGwaPlayerController"));
                OnResult.ExecuteIfBound(false, TEXT("Invalid PlayerController type"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AuthService: No PlayerController found"));
            OnResult.ExecuteIfBound(false, TEXT("No connection to server"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AuthService: No World context"));
        OnResult.ExecuteIfBound(false, TEXT("Invalid game state"));
    }
}

void UAuthService::RequestLogin(const FString& Username, const FString& Password, FLoginDelegate OnResult)
{
    UE_LOG(LogTemp, Log, TEXT("AuthService: Requesting login for user %s"), *Username);

    if (Username.IsEmpty() || Password.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthService: Invalid login parameters"));
        OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Username and password are required"));
        return;
    }

    // Get PlayerController and call server RPC
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            // Cast to our custom PlayerController that has authentication RPCs
            if (AGGwaPlayerController* GGwaPC = Cast<AGGwaPlayerController>(PC))
            {
                // Store delegate for later callback
                PendingLoginDelegate = OnResult;
                
                // Call Server RPC for login
                GGwaPC->Server_Login(Username, Password);
                
                UE_LOG(LogTemp, Log, TEXT("AuthService: Login request sent to server"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AuthService: PlayerController is not AGGwaPlayerController"));
                OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Invalid PlayerController type"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AuthService: No PlayerController found"));
            OnResult.ExecuteIfBound(false, TEXT(""), TEXT("No connection to server"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AuthService: No World context"));
        OnResult.ExecuteIfBound(false, TEXT(""), TEXT("Invalid game state"));
    }
}

// Called by PlayerController when server responds to registration
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

// Called by PlayerController when server responds to login
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

