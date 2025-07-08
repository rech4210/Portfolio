// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopSubsystem.h"

#include "DatabaseManager.h"
#include "ShopRepository.h"
#include "Components/ShopComponent.h"
#include "GameFramework/PlayerState.h"

void UShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	ShopRepository = NewObject<UShopRepository>(this, TEXT("ShopRepository"));
	ShopRepository->Initialize();
}

void UShopSubsystem::Deinitialize()
{
	ShopRepository = nullptr;
	Super::Deinitialize();
}

TScriptInterface<IShopRepositoryInterface> UShopSubsystem::GetShopRepository() const
{
	return ShopRepository;
}

void UShopSubsystem::RequestLoadShopData(APlayerState* PlayerState)
{
	// 클라이언트는 DB에서 데이터를 로드하지 않고 복제를 기다립니다.
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	if (ShopRepository && PlayerState)
	{
		// 서버에서만 실행: 비동기적으로 DB 또는 외부 저장소에서 상점 데이터 로드
		if (auto* ShopComponent = PlayerState->FindComponentByClass<UShopComponent>())
		{
			// 서버 권한이 있을 때만 ShopComponent의 복제된 프로퍼티를 수정
			if (PlayerState->HasAuthority())
			{
				// ShopRepository를 통해 상점 데이터 로드
				ShopRepository->LoadShopData(PlayerState->GetPlayerId(), *ShopComponent);
			}
		}
	}
}

void UShopSubsystem::Client_OnShopStateUpdated(UShopComponent* ShopComponent)
{
	if (ShopRepository && ShopComponent)
	{
		// 클라이언트 측에서 복제된 데이터를 받았을 때의 로직
		// 로컬 데이터 에셋이나 캐시를 로드·적용하고, 필요한 로직을 수행
		
		UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Client received shop state update for %d items"), 
			ShopComponent->GetAllShopItems().Num());
	}
}
