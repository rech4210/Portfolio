
#include "EquipmentRepository.h"
#include "Components/EquipmentComponent.h"
#include "DatabaseModule/Public/DatabaseManager.h"

void UEquipmentRepository::Initialize(IDBProviderInfra* Infra) {
	DBManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if (!DBManager) {
		UE_LOG(LogTemp, Error, TEXT("EquipmentRepo : DatabaseManager is not available!"));
	}
}

bool UEquipmentRepository::LoadEquipmentData(const FGuid& PlayerGuid, UEquipmentComponent& EquipmentComponentToPopulate)
{
	if (!EquipmentComponentToPopulate.GetOwner() || !EquipmentComponentToPopulate.GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: LoadEquipmentData can only be called on server authority"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Loading equipment data for Player %s"), *PlayerGuid.ToString());

	TArray<FEquipmentSlotState> MockEquipmentItems;
	
	FEquipmentSlotState Weapon;
	Weapon.ItemID = 3001;
	Weapon.SlotIndex = 0;
	Weapon.SlotType = TEXT("Weapon");
	Weapon.bIsEquipped = true;
	Weapon.EnhancementLevel = {5, 2, 1};
	MockEquipmentItems.Add(Weapon);
	
	FEquipmentSlotState Armor;
	Armor.ItemID = 3002;
	Armor.SlotIndex = 1;
	Armor.SlotType = TEXT("Armor");
	Armor.bIsEquipped = true;
	Armor.EnhancementLevel = {3, 1};
	MockEquipmentItems.Add(Armor);
	
	FEquipmentSlotState Accessory;
	Accessory.ItemID = 3003;
	Accessory.SlotIndex = 2;
	Accessory.SlotType = TEXT("Accessory");
	Accessory.bIsEquipped = true;
	Accessory.EnhancementLevel = {7};
	MockEquipmentItems.Add(Accessory);

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

	return true;
}
