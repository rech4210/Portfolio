#include "UI/UIManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Shared/Cache/UIConfigCacheActor.h"
#include "Shared/GameState/GGwaGameState.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 의존관계 역전 (IOC) 패턴을 사용하여 GameState에게 Client UIManagerSubsystem을 제공하도록 요청.
	if (UWorld* World = GetWorld())
	{
		if (AGGwaGameState * GameState = Cast<AGGwaGameState>(World->GetGameState())) {
			GameState->InitializeUISubsystemWithIOC(this);
		}
	}
	// Register for world subsystem events instead of world delegates
	// This is more reliable and doesn't have the compilation issue
	
	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Initialized - will listen for cache actor from GameState"));
}

void UUIManagerSubsystem::Deinitialize()
{
	// Clean up current widget
	RemoveCurrentWidget();

	// Clear cache actor reference
	CacheActor = nullptr;

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Deinitialized"));

	Super::Deinitialize();
}

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
