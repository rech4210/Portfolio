// @Needmodifi
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
	// ?´ë¼?´ì–¸?¸ëŠ” DB?ì„œ ?°ì´?°ë? ë¡œë“œ?˜ì? ?Šê³  ë³µì œë¥?ê¸°ë‹¤ë¦½ë‹ˆ??
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
			// ?œë²„?ì„œë§??¤í–‰: ë¹„ë™ê¸°ì ?¼ë¡œ DB ?ëŠ” ?¸ë? ?€?¥ì†Œ?ì„œ ?¥ë¹„ ?°ì´??ë¡œë“œ
			if (auto* EquipmentComponent = PlayerState->FindComponentByClass<UEquipmentComponent>())
			{
				// ?œë²„ ê¶Œí•œ???ˆì„ ?Œë§Œ EquipmentComponent??ë³µì œ???„ë¡œ?¼í‹°ë¥??˜ì •
				if (PlayerState->HasAuthority())
				{
					// EquipmentRepositoryë¥??µí•´ ?¥ë¹„ ?°ì´??ë¡œë“œ
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
		// ?´ë¼?´ì–¸??ì¸¡ì—??ë³µì œ???°ì´?°ë? ë°›ì•˜???Œì˜ ë¡œì§
		// ë¡œì»¬ ?°ì´???ì…‹?´ë‚˜ ìºì‹œë¥?ë¡œë“œÂ·?ìš©?˜ê³ , ?„ìš”??ë¡œì§???˜í–‰
		
		UE_LOG(LogTemp, Log, TEXT("EquipmentSubsystem: Client received equipment state update for %d items"), 
			EquipmentComponent->GetAllEquipment().Num());
	}
}
