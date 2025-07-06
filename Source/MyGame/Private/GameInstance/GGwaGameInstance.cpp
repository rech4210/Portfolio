// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/GGwaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UGGwaGameInstance::UGGwaGameInstance()
{
}

void UGGwaGameInstance::Init() {
	Super::Init();

	// 맵 로딩은 Init 단계에서는 아직 World가 완전히 초기화되지 않았을 수 있어서,
	// 약간의 지연을 줘서 안전하게 처리
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGGwaGameInstance::HandleMapLoading, 0.1f, false);
}

void UGGwaGameInstance::HandleMapLoading() {
	if (!GetWorld()) return;

	// 서버 측이면 MainMap으로 이동
	if (IsDedicatedServerInstance() || GetWorld()->GetNetMode()==NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버 인스턴스입니다. MainMap으로 이동합니다."));
		GetWorld()->ServerTravel(TEXT("/Game/Map/ThirdPersonMap?listen"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("클라이언트 인스턴스입니다. LoginMap에 머무릅니다."));
		// 클라이언트는 LoginMap에 머무름 (필요하면 UI 표시 등 추가 가능)
	}
}
