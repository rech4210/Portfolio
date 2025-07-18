#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Interface/UISubsystemInterface.h"
#include "UIManagerSubsystem.generated.h"

class AUIConfigCacheActor;

/**
 * UI Manager Subsystem for automatic widget creation/removal based on map changes
 * Manages UI lifecycle automatically when transitioning between different maps
 * Now works with replicated UIConfigCacheActor for server-controlled UI configuration
 */
UCLASS(BlueprintType, Blueprintable)
class CLIENTMODULE_API UUIManagerSubsystem : public UGameInstanceSubsystem, public IUISubsystemInterface
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	/**
	 * Currently active widget instance
	 * Cached to allow proper cleanup when transitioning between maps
	 */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentWidget;

	/**
	 * Reference to the replicated cache actor
	 * Contains server-controlled map-widget mappings
	 */
	UPROPERTY(Transient)
	TObjectPtr<AUIConfigCacheActor> CacheActor;

	/**
	 * Called when a new map is loaded
	 * Automatically creates appropriate UI for the loaded map
	 */
	UFUNCTION()
	void OnMapChanged(UWorld* LoadedWorld);

public:
	/**
	 * Set the cache actor reference (called by GameState)
	 * This replaces the old local MapWidgetMap functionality
	 */
	// UFUNCTION(BlueprintCallable, Category = "UI Manager")
	virtual void SetCacheActor(AUIConfigCacheActor* NewCacheActor) override;

	/**
	 * Request to set widget for a specific map through cache actor
	 * This will send request to replicated cache actor
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RequestSetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass);

	/**
	 * Get currently active widget
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Manager")
	UUserWidget* GetCurrentWidget() const { return CurrentWidget; }

	/**
	 * Get the cache actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Manager")
	AUIConfigCacheActor* GetCacheActor() const { return CacheActor; }

	/**
	 * Force refresh current widget (useful for debugging)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RefreshCurrentWidget();

	/**
	 * Manually remove current widget
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RemoveCurrentWidget();
};
