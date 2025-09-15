
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
	// EquipmentRepository->Initialize(TODO); Infra 주입으로 변경
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
			if (auto* EquipmentComponent = PlayerState->FindComponentByClass<UEquipmentComponent>())
			{
				if (PlayerState->HasAuthority())
				{
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
		UE_LOG(LogTemp, Log, TEXT("EquipmentSubsystem: Client received equipment state update for %d items"), 
			EquipmentComponent->GetAllEquipment().Num());
	}
}
