#include "Shared/Cache/UIConfigCacheActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AUIConfigCacheActor::AUIConfigCacheActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Enable replication
	bReplicates = true;
	bAlwaysRelevant = true;
	
	// Set replication frequency for immediate updates
	SetNetUpdateFrequency(10.0f);
	SetMinNetUpdateFrequency(1.0f);
}

void AUIConfigCacheActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate MapWidgetMap with OnRep function
	DOREPLIFETIME_CONDITION_NOTIFY(AUIConfigCacheActor, WidgetConifgs, COND_None, REPNOTIFY_Always);
}

void AUIConfigCacheActor::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("UIConfigCacheActor::BeginPlay - HasAuthority: %s, MapWidgetMap entries: %d"), 
		HasAuthority() ? TEXT("true") : TEXT("false"), WidgetConifgs.Num());
}

void AUIConfigCacheActor::SetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("UIConfigCacheActor::SetWidgetForMap called on client - ignoring"));
		return;
	}

	if (WidgetClass)
	{
		WidgetConifgs.Add(FMapWidgetConifg(MapName, WidgetClass));
		UE_LOG(LogTemp, Log, TEXT("UIConfigCacheActor::SetWidgetForMap - Added mapping: %s -> %s"), 
			*MapName.ToString(), *WidgetClass->GetName());
	}

	// Force replication update
	ForceNetUpdate();
}

TSubclassOf<UUserWidget> AUIConfigCacheActor::GetWidgetForMap(FName MapName) const
{
	if (const FMapWidgetConifg* Found = WidgetConifgs.FindByPredicate([&](FMapWidgetConifg& It){return It.Key == MapName;}))
	{
		return Found->WidgetClass;
	}

	return nullptr;
}

void AUIConfigCacheActor::InitializeDefaultMappings()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("UIConfigCacheActor::InitializeDefaultMappings called on client - ignoring"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UIConfigCacheActor::InitializeDefaultMappings - Setting up default UI mappings"));

	// TODO: Configure default mappings here or load from data table
	// Example:
	// SetWidgetForMap(FName("LoginLevel"), LoginWidgetClass);
	// SetWidgetForMap(FName("GameLevel"), GameHUDClass);
	// SetWidgetForMap(FName("MenuLevel"), MainMenuClass);

	UE_LOG(LogTemp, Log, TEXT("UIConfigCacheActor::InitializeDefaultMappings - Completed with %d mappings"), 
		WidgetConifgs.Num());
}

void AUIConfigCacheActor::OnRep_MapWidgetMap()
{
	UE_LOG(LogTemp, Log, TEXT("UIConfigCacheActor::OnRep_MapWidgetMap - Received %d map-widget mappings from server"), 
		WidgetConifgs.Num());

	// Log all received mappings for debugging
	for (const auto& Pair : WidgetConifgs)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("  Map: %s -> Widget: %s"), 
			*Pair.Key.ToString(), 
			Pair.WidgetClass ? *Pair.WidgetClass->GetName() : TEXT("NULL"));
	}
}
