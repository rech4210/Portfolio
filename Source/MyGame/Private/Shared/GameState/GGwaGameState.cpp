#include "Shared/GameState/GGwaGameState.h"
#include "Shared/Cache/UIConfigCacheActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

// #if !UE_SERVER
// #include "ClientModule/Public/UI/UIManagerSubsystem.h"
// #endif

AGGwaGameState::AGGwaGameState()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Enable replication
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AGGwaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate CacheActor to all clients
	DOREPLIFETIME(AGGwaGameState, CacheActor);
}

void AGGwaGameState::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("GGwaGameState::BeginPlay - HasAuthority: %s"), 
		HasAuthority() ? TEXT("true") : TEXT("false"));
}

void AGGwaGameState::SetCacheActor(AUIConfigCacheActor* NewCacheActor)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("GGwaGameState::SetCacheActor called on client - ignoring"));
		return;
	}

	CacheActor = NewCacheActor;
	UE_LOG(LogTemp, Log, TEXT("GGwaGameState::SetCacheActor - Cache actor set on server"));

	// Notify all clients immediately
	Multicast_InitCacheActor(NewCacheActor);
}

void AGGwaGameState::Multicast_InitCacheActor_Implementation(AUIConfigCacheActor* NewActor)
{
	UE_LOG(LogTemp, Log, TEXT("GGwaGameState::Multicast_InitCacheActor - Received cache actor: %s"), 
		NewActor ? *NewActor->GetName() : TEXT("NULL"));

#if !UE_SERVER
	// Notify UIManagerSubsystem about the cache actor
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UISubsystemInterface)
		{
			UISubsystemInterface->SetCacheActor(NewActor);
			UE_LOG(LogTemp, Log, TEXT("GGwaGameState::Multicast_InitCacheActor - UIManagerSubsystem notified"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GGwaGameState::Multicast_InitCacheActor - UIManagerSubsystem not found"));
		}
	}
#else
	UE_LOG(LogTemp, Log, TEXT("GGwaGameState::Multicast_InitCacheActor - Server received multicast (expected)"));
#endif
}
