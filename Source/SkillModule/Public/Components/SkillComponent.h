// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "SkillComponent.generated.h"

class USkillSlot;
class USkillDataAsset;
class UGameplayAbility;
struct FSkillDomain;

// Domain Events for SkillComponent (Aggregate)
// DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillRegistered, USkillDataAsset* /* SkillData */);
// DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillUnregistered, const FGuid& /* SlotId */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillsSwapped, const FGuid& /* SlotIdA */, const FGuid& /* SlotIdB */);
DECLARE_MULTICAST_DELEGATE(FOnSkillsChanged);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillStateChanged, const TArray<USkillSlot*>&);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillStateChanged, const TArray<USkillSlot*>&, SkillSlots);
/**
 * 캐릭터의 스킬 슬롯들을 관리하는 Aggregate Root 역할을 하는 컴포넌트입니다.
 * DDD 원칙에 따라 불변 조건을 보장하고 도메인 이벤트를 발행합니다.
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SKILLMODULE_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USkillComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetMaxSlotCount() const { return MaxSkillSlots; }

	// 스킬 상태가 변경될 때 호출되는 이벤트 (등록, 제거, 스왑 등)
	// UPROPERTY(BlueprintAssignable, Category = "Skill|Events")
	FOnSkillStateChanged OnSkillStateChanged;

	// Domain Events (for DDD compliance)
	// FOnSkillRegistered OnSkillRegistered;
	// FOnSkillUnregistered OnSkillUnregistered;
	FOnSkillsSwapped OnSkillsSwapped;
	FOnSkillsChanged OnSkillsChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_SkillSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Component")
	TArray<TObjectPtr<USkillSlot>> SkillSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Component")
	int32 MaxSkillSlots = 8;

public:
	// ========================================================================
	// AGGREGATE ROOT METHODS - BUSINESS LOGIC WITH INVARIANT PROTECTION
	// ========================================================================

	/**
	 * Register a skill to available slot (Domain logic with validation)
	 * @param SkillData Skill data to register
	 * @return True if successfully registered
	 */
	bool RegisterSkill(USkillDataAsset* SkillData);

	/**
	 * Unregister a skill from slot (Domain logic with validation)
	 * @param SlotId Slot ID to unregister
	 */
	void UnregisterSkill(const FGuid& SlotId);

	/**
	 * Swap skills between two slots (Domain logic with validation)
	 * @param SlotIdA First slot ID
	 * @param SlotIdB Second slot ID
	 */
	void SwapSkills(const FGuid& SlotIdA, const FGuid& SlotIdB);

	/**
	 * Server-side method to set skill slots (used by Repository)
	 * @param InSkillSlots Array of skill slots to set
	 */
	void Server_SetSkillSlots(const TArray<USkillSlot*>& InSkillSlots);

	// ========================================================================
	// DOMAIN LOGIC METHODS - BUSINESS RULES VALIDATION
	// ========================================================================

	/**
	 * Validate if a skill can be registered (Domain Rule)
	 * @param SkillData Skill data to validate
	 * @return True if skill can be registered
	 */
	bool CanRegisterSkill(USkillDataAsset* SkillData) const;

	/**
	 * Validate if a skill can be unregistered (Domain Rule)
	 * @param SlotId Slot ID to validate
	 * @return True if skill can be unregistered
	 */
	bool CanUnregisterSkill(const FGuid& SlotId) const;

	/**
	 * Validate if skills can be swapped (Domain Rule)
	 * @param SlotIdA First slot ID
	 * @param SlotIdB Second slot ID
	 * @return True if skills can be swapped
	 */
	bool CanSwapSkills(const FGuid& SlotIdA, const FGuid& SlotIdB) const;

	/**
	 * Validate if cooldown can be updated (Domain Rule)
	 * @param SlotId Slot ID to validate
	 * @param LastUsedTime The last used time
	 * @param RemainingCooldown The remaining cooldown
	 * @return True if cooldown can be updated
	 */
	bool CanUpdateCooldown(const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown) const;

	/**
	 * Validate if skills can be saved (Domain Rule)
	 * @param SkillData Skill domain data to validate
	 * @return True if skills can be saved
	 */
	bool CanSaveSkills(const FSkillDomain& SkillData) const;

	/**
	 * Check if player has a specific skill
	 * @param SkillData Skill data to check
	 * @return True if player has this skill
	 */
	bool HasSkill(USkillDataAsset* SkillData) const;

	/**
	 * Check if there are available skill slots
	 * @return True if there are available slots
	 */
	bool HasAvailableSlot() const;

	/**
	 * Get available slot index
	 * @return Available slot index, -1 if no slots available
	 */
	int32 GetAvailableSlotIndex() const;

	// ========================================================================
	// QUERY METHODS - READ-ONLY ACCESS
	// ========================================================================

	/**
	 * Get skill slot by GUID
	 * @param SlotId Slot ID to find
	 * @return Found skill slot, nullptr if not found
	 */
	USkillSlot* GetSkillSlotByGuid(const FGuid& SlotId) const;

	/**
	 * Get skill slot GUID by index
	 * @param index Slot index
	 * @return Slot GUID, invalid GUID if not found
	 */
	FGuid GetSkillSlotGuidByIndex(int32 index) const;

	/**
	 * Get all skill slots (read-only access)
	 * @return Array of all skill slots
	 */
	const TArray<USkillSlot*>& GetAllSkillSlots() const { return reinterpret_cast<const TArray<USkillSlot*>&>(SkillSlots); }

	// ========================================================================
	// DOMAIN INTEGRATION METHODS - AGGREGATE SYNCHRONIZATION
	// ========================================================================

	/**
	 * Synchronize component state with domain data (called by DomainService)
	 * Updates the aggregate's internal state to match the domain data
	 * @param SkillData Domain data to sync with
	 */
	void SyncWithDomain(const FSkillDomain& SkillData);

	/**
	 * Extract current domain data from component (called by DomainService)
	 * Creates domain data representation from current aggregate state
	 * @return Current skill domain data
	 */
	FSkillDomain ExtractDomain() const;

	// 이미 복제를 수행중.
	// virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnRep_SkillSlots();

private:
	/**
	 * Internal method to notify skill state changes
	 */
	void NotifySkillStateChanged();
};