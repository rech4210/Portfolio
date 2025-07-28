// @Needmodifi
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "UIConfigCacheActor.generated.h"

/**
 * Replicated actor that caches UI configuration data
 * Contains map-to-widget mappings that are synchronized from server to all clients
 */

USTRUCT(BlueprintType)
struct FMapWidgetConifg{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="UI Config")
	FName Key;
	
	UPROPERTY(EditDefaultsOnly, Category="UI Config")
	TSubclassOf<UUserWidget> WidgetClass;
};
UCLASS(BlueprintType, Blueprintable)
class MYGAME_API AUIConfigCacheActor : public AActor
{
	GENERATED_BODY()

public:
	AUIConfigCacheActor();

	// UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * Map Widget Configuration (Replicated)
	 * Maps level names to their corresponding widget classes
	 * This data is set by server and replicated to all clients
	 */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "UI Config")
	TArray<FMapWidgetConifg> WidgetConifgs;

	/**
	 * Set widget class for a specific map (Server only)
	 * This will be replicated to all clients automatically
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Config")
	void SetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass);

	/**
	 * Get widget class for a specific map
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Config")
	TSubclassOf<UUserWidget> GetWidgetForMap(FName MapName) const;

	/**
	 * Get all configured map-widget pairs
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Config")
	const TArray<FMapWidgetConifg>& GetMapWidgetMap() const { return WidgetConifgs; }

	/**
	 * Initialize with default map-widget configurations (Server only)
	 * Called by GameMode during setup
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Config")
	void InitializeDefaultMappings();

protected:
	/**
	 * Called when MapWidgetMap is replicated to clients
	 */
	UFUNCTION()
	void OnRep_MapWidgetMap();
};
