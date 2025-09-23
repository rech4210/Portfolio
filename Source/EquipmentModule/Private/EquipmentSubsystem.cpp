#include "EquipmentSubsystem.h"
#include "EquipmentRepository.h"
#include "Components/EquipmentComponent.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"
#include "Provider/DBProviderInfra.h"

void UEquipmentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Collection.InitializeDependency(UDBProviderInfra::StaticClass());
	Super::Initialize(Collection);
	DefaultEquipmentRepository = NewObject<UEquipmentRepository>(this, TEXT("EquipmentRepository"));
	// EquipmentRepository->Initialize(TODO); Infra 주입으로 변경
}

void UEquipmentSubsystem::Deinitialize()
{
	DefaultEquipmentRepository = nullptr;
	Super::Deinitialize();
}

TScriptInterface<IEquipmentRepositoryInterface> UEquipmentSubsystem::GetEquipmentRepository() const
{
	return DefaultEquipmentRepository;
}

void UEquipmentSubsystem::RequestLoadEquipmentData(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity)
{
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	if (DefaultEquipmentRepository && PlayerIdentity)
	{
		UObject* PlayerObject = Cast<UObject>(PlayerIdentity.GetObject());
		APlayerState* PlayerState = Cast<APlayerState>(PlayerObject);
		if (PlayerState)
		{
			if (auto* EquipmentComponent = PlayerState->FindComponentByClass<UEquipmentComponent>())
			{
				if (PlayerState->HasAuthority())
				{
					DefaultEquipmentRepository->LoadEquipmentData(PlayerIdentity->GetPlayerGuid(), *EquipmentComponent);
				}
			}
		}
	}
}

void UEquipmentSubsystem::Client_OnEquipmentStateUpdated(UEquipmentComponent* EquipmentComponent)
{
	if (DefaultEquipmentRepository && EquipmentComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("EquipmentSubsystem: Client received equipment state update for %d items"), 
			EquipmentComponent->GetAllEquipment().Num());
	}
}
