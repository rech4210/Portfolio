#include "Shared/Service/ClientServiceManager.h"
#include "Engine/Engine.h"

UClientServiceManager::UClientServiceManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: ActorComponent constructor called"));
}

void UClientServiceManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: BeginPlay - Owner: %s"), 
		GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
}

// ============================================================================
// IClientServiceManagerInterface INTERFACE IMPLEMENTATION
// Service location and registration (GGwaGameState pattern)
// ============================================================================

IClientAuthInterface* UClientServiceManager::GetAuthService()
{
	return AuthServiceInterface.GetInterface();
}

IClientUIInterface* UClientServiceManager::GetUIService()
{
	return UIServiceInterface.GetInterface();
}

void UClientServiceManager::SetAuthService(TScriptInterface<IClientAuthInterface> AuthService)
{
	AuthServiceInterface = AuthService;
	
	UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: AuthService registered - Valid: %s"), 
		AuthServiceInterface.GetInterface() ? TEXT("true") : TEXT("false"));
}

void UClientServiceManager::SetUIService(TScriptInterface<IClientUIInterface> UIService)
{
	UIServiceInterface = UIService;
	
	UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: UIService registered - Valid: %s"), 
		UIServiceInterface.GetInterface() ? TEXT("true") : TEXT("false"));
}

bool UClientServiceManager::IsServiceReady() const
{
	bool bAuthReady = AuthServiceInterface.GetInterface() != nullptr;
	bool bUIReady = UIServiceInterface.GetInterface() != nullptr;
	
	return bAuthReady || bUIReady; // At least one service should be available
}

// ============================================================================
// IClientComponentProvider INTERFACE IMPLEMENTATION
// Legacy compatibility - delegates to service manager
// ============================================================================

// Authentication delegation
void UClientServiceManager::InitializeClientAuth()
{
	if (IClientAuthInterface* AuthService = GetAuthService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating auth initialization to AuthService"));
		AuthService->InitializeAuth();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientServiceManager: Cannot initialize auth - AuthService not registered"));
	}
}

void UClientServiceManager::RequestClientRegistration(const FString& Username, const FString& Password)
{
	if (IClientAuthInterface* AuthService = GetAuthService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating registration request to AuthService"));
		AuthService->RequestRegistration(Username, Password);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientServiceManager: Cannot request registration - AuthService not registered"));
	}
}

void UClientServiceManager::RequestClientLogin(const FString& Username, const FString& Password)
{
	if (IClientAuthInterface* AuthService = GetAuthService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating login request to AuthService"));
		AuthService->RequestLogin(Username, Password);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientServiceManager: Cannot request login - AuthService not registered"));
	}
}

void UClientServiceManager::OnServerRegistrationResult(bool bSuccess, const FString& Message)
{
	if (IClientAuthInterface* AuthService = GetAuthService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating registration result to AuthService"));
		AuthService->OnServerRegistrationResult(bSuccess, Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientServiceManager: Cannot handle registration result - AuthService not registered"));
	}
}

void UClientServiceManager::OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId)
{
	if (IClientAuthInterface* AuthService = GetAuthService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating login result to AuthService"));
		AuthService->OnServerLoginResult(bSuccess, Token, UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientServiceManager: Cannot handle login result - AuthService not registered"));
	}
}

// UI delegation
void UClientServiceManager::InitializeClientUI(const USkillComponent* SkillComponent)
{
	if (IClientUIInterface* UIService = GetUIService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating UI initialization to UIService"));
		UIService->InitializeUI(SkillComponent);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Cannot initialize UI - UIService not registered (normal for server)"));
	}
}

void UClientServiceManager::HandleClientMouseOverDetection()
{
	if (IClientUIInterface* UIService = GetUIService())
	{
		UIService->HandleMouseOverDetection();
	}
	// Note: No warning here as this is called frequently
}

void UClientServiceManager::NotifyClientStateChanged()
{
	if (IClientUIInterface* UIService = GetUIService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating state change notification to UIService"));
		UIService->NotifyStateChanged();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Cannot notify state change - UIService not registered"));
	}
}

void UClientServiceManager::ReceiveBossDataFromServer(const FBossDataStruct& BossData)
{
	if (IClientUIInterface* UIService = GetUIService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating boss data to UIService"));
		UIService->ReceiveBossData(BossData);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Cannot handle boss data - UIService not registered"));
	}
}

void UClientServiceManager::ReceiveSkillDataFromServer(const USkillComponent* SkillComponent)
{
	if (IClientUIInterface* UIService = GetUIService())
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Delegating skill data to UIService"));
		UIService->ReceiveSkillData(SkillComponent);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ClientServiceManager: Cannot handle skill data - UIService not registered"));
	}
}
