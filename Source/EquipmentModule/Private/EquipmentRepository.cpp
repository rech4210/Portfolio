// @Needmodifi
// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentRepository.h"
#include "Components/EquipmentComponent.h"
#include "DatabaseModule/Public/DatabaseManager.h"

void UEquipmentRepository::Initialize() {
	DBManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if (!DBManager) {
		UE_LOG(LogTemp, Error, TEXT("EquipmentRepo : DatabaseManager is not available!"));
	}
}

bool UEquipmentRepository::LoadEquipmentData(const FGuid& PlayerGuid, UEquipmentComponent& EquipmentComponentToPopulate)
{
	// ?œë²„ ê¶Œí•œ???ˆì„ ?Œë§Œ ë³µì œ???„ë¡œ?¼í‹°ë¥??˜ì •
	if (!EquipmentComponentToPopulate.GetOwner() || !EquipmentComponentToPopulate.GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: LoadEquipmentData can only be called on server authority"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Loading equipment data for Player %s"), *PlayerGuid.ToString());

	// Mock ?¥ë¹„ ?°ì´???ì„± (?¤ì œë¡œëŠ” DB?ì„œ ë¡œë“œ)
	TArray<FEquipmentSlotState> MockEquipmentItems;
	
	// ë¬´ê¸° ?¬ë¡¯
	FEquipmentSlotState Weapon;
	Weapon.ItemID = 3001;
	Weapon.SlotIndex = 0;
	Weapon.SlotType = TEXT("Weapon");
	Weapon.bIsEquipped = true;
	Weapon.EnhancementLevel = {5, 2, 1}; // ê°•í™” ?ˆë²¨ ?•ë³´
	MockEquipmentItems.Add(Weapon);
	
	// ë°©ì–´êµ??¬ë¡¯
	FEquipmentSlotState Armor;
	Armor.ItemID = 3002;
	Armor.SlotIndex = 1;
	Armor.SlotType = TEXT("Armor");
	Armor.bIsEquipped = true;
	Armor.EnhancementLevel = {3, 1};
	MockEquipmentItems.Add(Armor);
	
	// ?¡ì„¸?œë¦¬ ?¬ë¡¯
	FEquipmentSlotState Accessory;
	Accessory.ItemID = 3003;
	Accessory.SlotIndex = 2;
	Accessory.SlotType = TEXT("Accessory");
	Accessory.bIsEquipped = true;
	Accessory.EnhancementLevel = {7};
	MockEquipmentItems.Add(Accessory);

	// ?¥ë¹„ ?„ì´?œë“¤??ì»´í¬?ŒíŠ¸??ì¶”ê? (ë³µì œ ?¸ë¦¬ê±?
	for (const auto& Equipment : MockEquipmentItems)
	{
		if (!EquipmentComponentToPopulate.EquipItem(Equipment))
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: Failed to equip item %d"), Equipment.ItemID);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Successfully equipped item %d in slot %d"), Equipment.ItemID, Equipment.SlotIndex);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Completed loading equipment data for Player %s"), *PlayerGuid.ToString());
	return true;
}

bool UEquipmentRepository::SaveEquipmentData(const FGuid& PlayerGuid, const UEquipmentComponent* EquipmentComponentToSave)
{
	if (!EquipmentComponentToSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: SaveEquipmentData called with null EquipmentComponent"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Saving equipment data for Player %s with %d items"), 
		*PlayerGuid.ToString(), EquipmentComponentToSave->GetAllEquipment().Num());

	// ?¤ì œ êµ¬í˜„?ì„œ???¬ê¸°???°ì´?°ë² ?´ìŠ¤???€??
	// ?„ì¬??Mock êµ¬í˜„
	
	return true;
}
