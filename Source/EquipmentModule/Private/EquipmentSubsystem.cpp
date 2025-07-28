
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
	// ?�라?�언?�는 DB?�서 ?�이?��? 로드?��? ?�고 복제�?기다립니??
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
			// ?�버?�서�??�행: 비동기적?�로 DB ?�는 ?��? ?�?�소?�서 ?�비 ?�이??로드
			if (auto* EquipmentComponent = PlayerState->FindComponentByClass<UEquipmentComponent>())
			{
				// ?�버 권한???�을 ?�만 EquipmentComponent??복제???�로?�티�??�정
				if (PlayerState->HasAuthority())
				{
					// EquipmentRepository�??�해 ?�비 ?�이??로드
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
		// ?�라?�언??측에??복제???�이?��? 받았???�의 로직
		// 로컬 ?�이???�셋?�나 캐시�?로드·?�용?�고, ?�요??로직???�행
		
		UE_LOG(LogTemp, Log, TEXT("EquipmentSubsystem: Client received equipment state update for %d items"), 
			EquipmentComponent->GetAllEquipment().Num());
	}
}
