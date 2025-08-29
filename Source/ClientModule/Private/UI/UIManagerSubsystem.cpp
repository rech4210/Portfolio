
#include "UI/UIManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Player/ClientAuthComponent.h"
#include "Player/ClientUIComponent.h"
#include "Shared/Cache/UIConfigCacheActor.h"
#include "Shared/GameState/GGwaGameState.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "SkillModule/Public/Components/SkillComponent.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::Initialize - Starting initialization"));
	
}
void UUIManagerSubsystem::OnWorldInit(UWorld* NewWorld, const UWorld::InitializationValues IVS) {
	if (!NewWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("UIManagerSubsystem::Initialize - World is null!"));
		return;
	}

	AGGwaGameState* GameState = Cast<AGGwaGameState>(GetWorld()->GetGameState());
	if (!GameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem::Initialize - GameState not available yet"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::Initialize - Found GameState: %s"), 
		*GameState->GetClass()->GetName());
	
	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::Initialize - Registered both interfaces with GameState"));
}


void UUIManagerSubsystem::Deinitialize()
{
	RemoveCurrentWidget();
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

	FName CurrentMapName = FName(*UGameplayStatics::GetCurrentLevelName(LoadedWorld));

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::OnMapChanged: Map changed to %s"), *CurrentMapName.ToString());

	RemoveCurrentWidget();

	if (!CacheActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem::OnMapChanged: No cache actor available"));
		return;
	}

	TSubclassOf<UUserWidget> WidgetClass = CacheActor->GetWidgetForMap(CurrentMapName);
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem::OnMapChanged: No widget configured for map %s"), *CurrentMapName.ToString());
		return;
	}

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

void UUIManagerSubsystem::RegistClientComponent(UActorComponent* Component) {
	if (auto Auth = Cast<UClientAuthComponent>(Component)){
		ClientAuthService = Auth;
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Registered ClientAuthComponent"));
	}
	else if (auto UI = Cast<UClientUIComponent>(Component)){
		ClientUIService = UI;
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Registered ClientUIComponent"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: Component is not a valid ClientAuth or ClientUI component"));
	}
}

void UUIManagerSubsystem::ProcessRegistration(const FString& Username, const FString& Password)
{
	if (ClientAuthService)
	{
		ClientAuthService->RequestRegistration(Username, Password);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing registration for user: %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for registration"));
	}
}

void UUIManagerSubsystem::ProcessLogin(const FString& Username, const FString& Password)
{
	if (ClientAuthService && IsValid(ClientAuthService))
	{
		ClientAuthService->RequestLogin(Username, Password);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing login for user: %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No valid auth service registered for login"));
	}
}

void UUIManagerSubsystem::HandleRegistrationResult(bool bSuccess, const FString& Message)
{
	if (ClientAuthService)
	{
		ClientAuthService->OnServerRegistrationResult(bSuccess, Message);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Handled registration result - Success: %s"), bSuccess ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for registration result"));
	}
}

void UUIManagerSubsystem::HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId)
{
	if (ClientAuthService)
	{
		ClientAuthService->OnServerLoginResult(bSuccess, Token, UserId);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Handled login result - Success: %s, UserId: %s"), 
			bSuccess ? TEXT("true") : TEXT("false"), *UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No auth service registered for login result"));
	}
}

void UUIManagerSubsystem::InitializeUI()
{
	if (ClientUIService)
	{
		ClientUIService->InitializeUI();
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Delegated InitializeUI"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered"));
	}
}

void UUIManagerSubsystem::ProcessMouseOverDetection()
{
	if (ClientUIService)
	{
		ClientUIService->HandleMouseOverDetection();
	}
	else
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("UIManagerSubsystem: No UI service registered for mouse detection"));
	}
}

void UUIManagerSubsystem::NotifyClientEvent(FGameplayTag Tag)
{
	if (ClientUIService)
	{
		ClientUIService->NotifyClientEvent(Tag);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Delegated NotifyStateChanged"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered for state change"));
	}
}

void UUIManagerSubsystem::ProcessBossData(const FBossDataStruct& BossData)
{
	if (ClientUIService)
	{
		ClientUIService->ReceiveBossData(BossData);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing boss data"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered for boss data"));
	}
}

void UUIManagerSubsystem::SkillHUDReplication(const FSkillSlotReplicationArray& SkillSlotsReplication)
{
	if (ClientUIService)
	{
		ClientUIService->ReceiveSkillReplicationData(SkillSlotsReplication);
		UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem: Processing skill replication data with %d items"), 
			SkillSlotsReplication.Items.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSubsystem: No UI service registered for skill replication data"));
	}
}

bool UUIManagerSubsystem::IsServiceReady() const
{
	bool bAuthReady = ClientAuthService != nullptr;
	bool bUIReady = ClientUIService != nullptr;
	
	UE_LOG(LogTemp, VeryVerbose, TEXT("UIManagerSubsystem service status - Auth: %s, UI: %s"), 
		bAuthReady ? TEXT("Ready") : TEXT("Not Ready"), 
		bUIReady ? TEXT("Ready") : TEXT("Not Ready"));
		
	return bAuthReady && bUIReady;
}
