#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameSharedModule/Public/Interface/UISubsystemInterface.h"
#include "GGwaGameState.generated.h"


class AUIConfigCacheActor;

UCLASS(BlueprintType, Blueprintable)
class MYGAME_API AGGwaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGGwaGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Cache")
	TObjectPtr<AUIConfigCacheActor> CacheActor;

	UFUNCTION(BlueprintCallable, Category = "Cache")
	void SetCacheActor(AUIConfigCacheActor* NewCacheActor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InitCacheActor(AUIConfigCacheActor* NewActor);

	void InitializeUISubsystemWithIOC(IUISubsystemInterface* Interface){
		UISubsystemInterface = Interface;
		UE_LOG(LogTemp, Log, TEXT("GGwaGameState::InitializeUISubsystemWithIOC - Interface registered"));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cache")
	AUIConfigCacheActor* GetCacheActor() const { return CacheActor; }

private:
	IUISubsystemInterface* UISubsystemInterface;
};
