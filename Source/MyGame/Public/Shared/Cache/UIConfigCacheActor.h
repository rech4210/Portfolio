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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "UI Config")
	TArray<FMapWidgetConifg> WidgetConifgs;

	UFUNCTION(BlueprintCallable, Category = "UI Config")
	void SetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Config")
	TSubclassOf<UUserWidget> GetWidgetForMap(FName MapName) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Config")
	const TArray<FMapWidgetConifg>& GetMapWidgetMap() const { return WidgetConifgs; }

	UFUNCTION(BlueprintCallable, Category = "UI Config")
	void InitializeDefaultMappings();

protected:

	UFUNCTION()
	void OnRep_MapWidgetMap();
};
