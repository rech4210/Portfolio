// @Needmodifi
// Fill out your copyright notice in the Description page of Project Settings.

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
	TArray<int32> EnhancementLevel; // Í∞ïÌôî ?àÎ≤® ??
};

/**
 * ?•ÎπÑ ?ÑÏù¥?úÎì§??Í¥ÄÎ¶¨Ìïò??Ïª¥Ìè¨?åÌä∏?ÖÎãà??
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentStateChanged, const TArray<FEquipmentSlotState>&, UpdatedEquipment);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EQUIPMENTMODULE_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEquipmentComponent();

	// ?•ÎπÑ ?ÅÌÉúÍ∞Ä Î≥ÄÍ≤ΩÎê† ???∏Ï∂ú?òÎäî ?¥Î≤§??
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnEquipmentStateChanged OnEquipmentStateChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_EquipmentSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Component")
	TArray<FEquipmentSlotState> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Component")
	int32 MaxEquipmentSlots = 10;

public:
	// ?•ÎπÑ Í¥ÄÎ¶?
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
