
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
	// ?�버 권한???�을 ?�만 복제???�로?�티�??�정
	if (!EquipmentComponentToPopulate.GetOwner() || !EquipmentComponentToPopulate.GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: LoadEquipmentData can only be called on server authority"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Loading equipment data for Player %s"), *PlayerGuid.ToString());

	// Mock ?�비 ?�이???�성 (?�제로는 DB?�서 로드)
	TArray<FEquipmentSlotState> MockEquipmentItems;
	
	// 무기 ?�롯
	FEquipmentSlotState Weapon;
	Weapon.ItemID = 3001;
	Weapon.SlotIndex = 0;
	Weapon.SlotType = TEXT("Weapon");
	Weapon.bIsEquipped = true;
	Weapon.EnhancementLevel = {5, 2, 1}; // 강화 ?�벨 ?�보
	MockEquipmentItems.Add(Weapon);
	
	// 방어�??�롯
	FEquipmentSlotState Armor;
	Armor.ItemID = 3002;
	Armor.SlotIndex = 1;
	Armor.SlotType = TEXT("Armor");
	Armor.bIsEquipped = true;
	Armor.EnhancementLevel = {3, 1};
	MockEquipmentItems.Add(Armor);
	
	// ?�세?�리 ?�롯
	FEquipmentSlotState Accessory;
	Accessory.ItemID = 3003;
	Accessory.SlotIndex = 2;
	Accessory.SlotType = TEXT("Accessory");
	Accessory.bIsEquipped = true;
	Accessory.EnhancementLevel = {7};
	MockEquipmentItems.Add(Accessory);

	// ?�비 ?�이?�들??컴포?�트??추�? (복제 ?�리�?
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

	// ?�제 구현?�서???�기???�이?�베?�스???�??
	// ?�재??Mock 구현
	
	return true;
}
