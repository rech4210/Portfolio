#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "GameSharedModule/Public/Interface/UISubsystemInterface.h"
#include "Shared/Interface/IClientComponentProvider.h"
#include "GGwaGameState.generated.h"


class AUIConfigCacheActor;

/**
 * Custom Game State for managing replicated game data
 * Handles UI configuration caching and other server-controlled state
 */
UCLASS(BlueprintType, Blueprintable)
class MYGAME_API AGGwaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGGwaGameState();

	// UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * Replicated cache actor for UI configuration
	 * Contains map-to-widget mappings that need to be synchronized to all clients
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Cache")
	TObjectPtr<AUIConfigCacheActor> CacheActor;

	/**
	 * Set the cache actor and notify all clients
	 * Called by GameMode when cache actor is spawned
	 */
	UFUNCTION(BlueprintCallable, Category = "Cache")
	void SetCacheActor(AUIConfigCacheActor* NewCacheActor);

	/**
	 * Multicast RPC to initialize cache actor on all clients
	 * Ensures UIManagerSubsystem gets the cache actor reference
	 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InitCacheActor(AUIConfigCacheActor* NewActor);

	void InitializeUISubsystemWithIOC(IUISubsystemInterface* Interface){
		UISubsystemInterface = Interface;
	}

	void InitializeClientManagerSubsystemWithIOC(TScriptInterface<IClientManagerInterface> Interface){
		if (!HasAuthority()) {
			ClientServiceInterface = Interface;
		}
	}

	TScriptInterface<IClientManagerInterface> GetClientManagerInterface() const {
		if (!ClientServiceInterface) return nullptr;
		return ClientServiceInterface;
	}

	/**
	 * Get the cache actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cache")
	AUIConfigCacheActor* GetCacheActor() const { return CacheActor; }

private:
	IUISubsystemInterface* UISubsystemInterface;
	TScriptInterface<IClientManagerInterface> ClientServiceInterface;
};
