// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentRepository.h"
#include "Components/EquipmentComponent.h"

bool UEquipmentRepository::LoadEquipmentData(int32 PlayerInformation, UEquipmentComponent& EquipmentComponentToPopulate)
{
	// 서버 권한이 있을 때만 복제된 프로퍼티를 수정
	if (!EquipmentComponentToPopulate.GetOwner() || !EquipmentComponentToPopulate.GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: LoadEquipmentData can only be called on server authority"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Loading equipment data for Player %d"), PlayerInformation);

	// Mock 장비 데이터 생성 (실제로는 DB에서 로드)
	TArray<FEquipmentSlotState> MockEquipmentItems;
	
	// 무기 슬롯
	FEquipmentSlotState Weapon;
	Weapon.ItemID = 3001;
	Weapon.SlotIndex = 0;
	Weapon.SlotType = TEXT("Weapon");
	Weapon.bIsEquipped = true;
	Weapon.EnhancementLevel = {5, 2, 1}; // 강화 레벨 정보
	MockEquipmentItems.Add(Weapon);
	
	// 방어구 슬롯
	FEquipmentSlotState Armor;
	Armor.ItemID = 3002;
	Armor.SlotIndex = 1;
	Armor.SlotType = TEXT("Armor");
	Armor.bIsEquipped = true;
	Armor.EnhancementLevel = {3, 1};
	MockEquipmentItems.Add(Armor);
	
	// 액세서리 슬롯
	FEquipmentSlotState Accessory;
	Accessory.ItemID = 3003;
	Accessory.SlotIndex = 2;
	Accessory.SlotType = TEXT("Accessory");
	Accessory.bIsEquipped = true;
	Accessory.EnhancementLevel = {7};
	MockEquipmentItems.Add(Accessory);

	// 장비 아이템들을 컴포넌트에 추가 (복제 트리거)
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
	
	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Completed loading equipment data for Player %d"), PlayerInformation);
	return true;
}

bool UEquipmentRepository::SaveEquipmentData(int32 PlayerInformation, const UEquipmentComponent* EquipmentComponentToSave)
{
	if (!EquipmentComponentToSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentRepository: SaveEquipmentData called with null EquipmentComponent"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("EquipmentRepository: Saving equipment data for Player %d with %d items"), 
		PlayerInformation, EquipmentComponentToSave->GetAllEquipment().Num());

	// 실제 구현에서는 여기서 데이터베이스에 저장
	// 현재는 Mock 구현
	
	return true;
}
