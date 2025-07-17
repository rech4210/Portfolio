// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSubsystem.h"

#include "DatabaseManager.h"
#include "EquipmentRepository.h"
#include "Components/EquipmentComponent.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"

void UEquipmentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);
	EquipmentRepository = NewObject<UEquipmentRepository>(this, TEXT("EquipmentRepository"));
	EquipmentRepository->Initialize();
}

void UEquipmentSubsystem::Deinitialize()
{
	EquipmentRepository = nullptr;
	Super::Deinitialize();
}

TScriptInterface<IEquipmentRepositoryInterface> UEquipmentSubsystem::GetEquipmentRepository() const
{
	return EquipmentRepository;
}

void UEquipmentSubsystem::RequestLoadEquipmentData(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity)
{
	// 클라이언트는 DB에서 데이터를 로드하지 않고 복제를 기다립니다.
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	if (EquipmentRepository && PlayerIdentity)
	{
		UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
		APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
		if (PlayerState)
		{
			// 서버에서만 실행: 비동기적으로 DB 또는 외부 저장소에서 장비 데이터 로드
			if (auto* EquipmentComponent = PlayerState->FindComponentByClass<UEquipmentComponent>())
			{
				// 서버 권한이 있을 때만 EquipmentComponent의 복제된 프로퍼티를 수정
				if (PlayerState->HasAuthority())
				{
					// EquipmentRepository를 통해 장비 데이터 로드
					EquipmentRepository->LoadEquipmentData(PlayerIdentity->GetPlayerGuid(), *EquipmentComponent);
				}
			}
		}
	}
}

void UEquipmentSubsystem::Client_OnEquipmentStateUpdated(UEquipmentComponent* EquipmentComponent)
{
	if (EquipmentRepository && EquipmentComponent)
	{
		// 클라이언트 측에서 복제된 데이터를 받았을 때의 로직
		// 로컬 데이터 에셋이나 캐시를 로드·적용하고, 필요한 로직을 수행
		
		UE_LOG(LogTemp, Log, TEXT("EquipmentSubsystem: Client received equipment state update for %d items"), 
			EquipmentComponent->GetAllEquipment().Num());
	}
}
