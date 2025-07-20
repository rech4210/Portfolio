#include "UI/UIManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Shared/Cache/UIConfigCacheActor.h"
#include "Shared/GameState/GGwaGameState.h"
#include "SkillModule/Public/Components/SkillComponent.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 의존관계 역전 (IOC) 패턴을 사용하여 GameState에게 Client UIManagerSubsystem을 제공하도록 요청.
	if (UWorld* World = GetWorld())
	{
		if (AGGwaGameState * GameState = Cast<AGGwaGameState>(World->GetGameState())) {
			GameState->InitializeUISubsystemWithIOC(this);
			GameState->InitializeClientManagerSubsystemWithIOC(this);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Initialized - will listen for cache actor from GameState"));
}

void UUIManagerSubsystem::Deinitialize()
{
	RemoveCurrentWidget();
	CacheActor = nullptr;

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Deinitialized"));

	Super::Deinitialize();
}

/* 기존 Subsystem에서 제공되던 함수를 모두 Client 측 Component의 역할로 위임시킬것.*/

void UUIManagerSubsystem::OnMapChanged(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem::OnMapChanged: LoadedWorld is null"));
		return;
	}

	// Get current level name
	FName CurrentMapName = FName(*UGameplayStatics::GetCurrentLevelName(LoadedWorld));

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::OnMapChanged: Map changed to %s"), *CurrentMapName.ToString());

	// Remove previous widget if exists
	RemoveCurrentWidget();

	// Check if we have cache actor with widget mappings
	if (!CacheActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem::OnMapChanged: No cache actor available"));
		return;
	}

	// Find widget class for current map from cache actor
	TSubclassOf<UUserWidget> WidgetClass = CacheActor->GetWidgetForMap(CurrentMapName);
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::OnMapChanged: No widget configured for map %s"), *CurrentMapName.ToString());
		return;
	}

	// Create new widget for current map
	CurrentWidget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
	if (CurrentWidget)
	{
		CurrentWidget->AddToViewport();
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::OnMapChanged: Created and added widget for map %s"), *CurrentMapName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UIManagerSubsystem::OnMapChanged: Failed to create widget for map %s"), *CurrentMapName.ToString());
	}
}

void UUIManagerSubsystem::SetCacheActor(AUIConfigCacheActor* NewCacheActor)
{
	CacheActor = NewCacheActor;
	
	if (CacheActor)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::SetCacheActor: Cache actor set with %d mappings"), 
			CacheActor->GetMapWidgetMap().Num());
		
		// Refresh current widget with new cache data
		RefreshCurrentWidget();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem::SetCacheActor: Cache actor set to null"));
	}
}

void UUIManagerSubsystem::RequestSetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!CacheActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem::RequestSetWidgetForMap: No cache actor available"));
		return;
	}

	// Delegate to cache actor (which will handle server authority)
	CacheActor->SetWidgetForMap(MapName, WidgetClass);
}

void UUIManagerSubsystem::RefreshCurrentWidget()
{
	if (UWorld* World = GetWorld())
	{
		OnMapChanged(World);
	}
}

void UUIManagerSubsystem::RemoveCurrentWidget()
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::RemoveCurrentWidget: Removed current widget"));
	}
}

// ============================================================================
// IClientUIManagerInterface IMPLEMENTATION 
// ============================================================================

void UUIManagerSubsystem::RegisterAuthService(TScriptInterface<IClientAuthInterface> AuthService)
{
	if (AuthService)
	{
		RegisteredAuthService = AuthService;
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Auth service registered"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: Failed to register auth service - null interface"));
	}
}

void UUIManagerSubsystem::RegisterUIService(TScriptInterface<IClientUIInterface> UIService)
{
	if (UIService)
	{
		RegisteredUIService = UIService;
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: UI service registered"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: Failed to register UI service - null interface"));
	}
}

// Auth service delegation
void UUIManagerSubsystem::InitializeAuth()
{
	if (RegisteredAuthService)
	{
		RegisteredAuthService->InitializeAuth();
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Delegated InitializeAuth"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered"));
	}
}

void UUIManagerSubsystem::ProcessRegistration(const FString& Username, const FString& Password)
{
	if (RegisteredAuthService)
	{
		RegisteredAuthService->RequestRegistration(Username, Password);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing registration for user: %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for registration"));
	}
}

void UUIManagerSubsystem::ProcessLogin(const FString& Username, const FString& Password)
{
	if (RegisteredAuthService)
	{
		RegisteredAuthService->RequestLogin(Username, Password);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing login for user: %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for login"));
	}
}

void UUIManagerSubsystem::HandleRegistrationResult(bool bSuccess, const FString& Message)
{
	if (RegisteredAuthService)
	{
		RegisteredAuthService->OnServerRegistrationResult(bSuccess, Message);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Handled registration result - Success: %s"), bSuccess ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for registration result"));
	}
}

void UUIManagerSubsystem::HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId)
{
	if (RegisteredAuthService)
	{
		RegisteredAuthService->OnServerLoginResult(bSuccess, Token, UserId);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Handled login result - Success: %s, UserId: %s"), 
			bSuccess ? TEXT("true") : TEXT("false"), *UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for login result"));
	}
}

// UI service delegation
void UUIManagerSubsystem::InitializeUI(const USkillComponent* SkillComponent)
{
	if (RegisteredUIService)
	{
		RegisteredUIService->InitializeUI(SkillComponent);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Delegated InitializeUI"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered"));
	}
}

void UUIManagerSubsystem::ProcessMouseOverDetection()
{
	if (RegisteredUIService)
	{
		RegisteredUIService->HandleMouseOverDetection();
		// No log here as this is called frequently
	}
	else
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("UIManagerSubsystem: No UI service registered for mouse detection"));
	}
}

void UUIManagerSubsystem::NotifyStateChanged()
{
	if (RegisteredUIService)
	{
		RegisteredUIService->NotifyStateChanged();
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Delegated NotifyStateChanged"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered for state change"));
	}
}

void UUIManagerSubsystem::ProcessBossData(const FBossDataStruct& BossData)
{
	if (RegisteredUIService)
	{
		RegisteredUIService->ReceiveBossData(BossData);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing boss data"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered for boss data"));
	}
}

void UUIManagerSubsystem::ProcessSkillData(const USkillComponent* SkillComponent)
{
	if (RegisteredUIService)
	{
		RegisteredUIService->ReceiveSkillData(SkillComponent);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing skill data"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered for skill data"));
	}
}

bool UUIManagerSubsystem::IsServiceReady() const
{
	bool bAuthReady = RegisteredAuthService != nullptr;
	bool bUIReady = RegisteredUIService != nullptr;
	
	UE_LOG(LogTemp, VeryVerbose, TEXT("UIManagerSubsystem service status - Auth: %s, UI: %s"), 
		bAuthReady ? TEXT("Ready") : TEXT("Not Ready"), 
		bUIReady ? TEXT("Ready") : TEXT("Not Ready"));
		
	return bAuthReady && bUIReady;
}
