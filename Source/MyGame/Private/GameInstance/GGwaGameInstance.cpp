
#include "GameInstance/GGwaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HAL/Platform.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"

UGGwaGameInstance::UGGwaGameInstance()
{
}

void UGGwaGameInstance::Init() {
	Super::Init();

	// �?로딩?� Init ?�계?�서???�직 World가 ?�전??초기?�되지 ?�았?????�어??
	// ?�간??지?�을 줘서 ?�전?�게 처리
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGGwaGameInstance::HandleMapLoading, 0.1f, false);
}

void UGGwaGameInstance::HandleMapLoading() {
	if (!GetWorld()) return;

	// PIE ?�경 �??�트?�크 모드 ?�확??감�?
	bool bIsPIEEnvironment = GetWorld()->WorldType == EWorldType::PIE;
	bool bIsActualDedicatedServer = IsDedicatedServerInstance();
	ENetMode NetMode = GetWorld()->GetNetMode();
	FString CurrentMapName = GetWorld()->GetMapName();
	
	UE_LOG(LogTemp, Warning, TEXT("=== GGwaGameInstance::HandleMapLoading ==="));
	UE_LOG(LogTemp, Warning, TEXT("PIE: %s, NetMode: %d, DedicatedServer: %s"), 
		bIsPIEEnvironment ? TEXT("Yes") : TEXT("No"),
		(int32)NetMode,
		bIsActualDedicatedServer ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Warning, TEXT("Current Map: %s"), *CurrentMapName);
	
	// Check if we're already in ThirdPersonMap (?�버가 ?�못??맵에???�작??경우)
	if (CurrentMapName.Contains(TEXT("ThirdPersonMap")))
	{
		UE_LOG(LogTemp, Error, TEXT("PROBLEM: Server started with ThirdPersonMap instead of LoginLevel!"));
		UE_LOG(LogTemp, Error, TEXT("This causes clients to auto-travel to ThirdPersonMap"));
		UE_LOG(LogTemp, Error, TEXT("Check PIE settings or Blueprint GameMode logic"));
	}

	// GameInstance??초기 ?�경 ?�정�?로깅�??�당
	// ?�제 �??�환 로직?� GameMode?�서 ?�레?�어 ???�을 고려?�여 처리
	
	if (bIsPIEEnvironment)
	{
		UE_LOG(LogTemp, Warning, TEXT("PIE Environment: LoginLevel�??�작, �??�라?�언?��? 개별 ?�증 ?�행"));
	}
	else if (bIsActualDedicatedServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dedicated Server: LoginLevel�??�작, GameMode?�서 ?�레?�어 ?�태???�라 �??�환 결정"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client/ListenServer: LoginLevel?�서 ?�증 ?�로?�스 ?�작"));
	}
	
	// Note: �??�환 로직?� AGGwaGameMode?�서 ?�레?�어 ?�속 ?�황??보고 결정
}
