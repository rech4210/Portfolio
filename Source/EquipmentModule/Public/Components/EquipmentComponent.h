
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UEquipmentItemData;

USTRUCT(BlueprintType)
struct FEquipmentSlotState
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	int32 SlotIndex = 0;

	UPROPERTY()
	FString SlotType; // "Weapon", "Armor", "Accessory" ??

	UPROPERTY()
	bool bIsEquipped = false;

	UPROPERTY()
	TArray<int32> EnhancementLevel; // 강화 ?�벨 ??
};

/**
 * ?�비 ?�이?�들??관리하??컴포?�트?�니??
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentStateChanged, const TArray<FEquipmentSlotState>&, UpdatedEquipment);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EQUIPMENTMODULE_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEquipmentComponent();

	// ?�비 ?�태가 변경될 ???�출?�는 ?�벤??
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnEquipmentStateChanged OnEquipmentStateChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_EquipmentSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Component")
	TArray<FEquipmentSlotState> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Component")
	int32 MaxEquipmentSlots = 10;

public:
	// ?�비 관�?
	bool EquipItem(const FEquipmentSlotState& EquipmentState);
	bool UnequipItem(int32 SlotIndex);
	bool UpdateEquipmentEnhancement(int32 SlotIndex, const TArray<int32>& NewEnhancementLevel);
	
	const TArray<FEquipmentSlotState>& GetAllEquipment() const { return EquipmentSlots; }
	FEquipmentSlotState* GetEquipmentSlot(int32 SlotIndex);
	int32 GetMaxSlotCount() const { return MaxEquipmentSlots; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_EquipmentSlots();
};
