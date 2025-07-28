#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Mappers/ISkillDtoMapper.h"
#include "Mappers/ISkillAssetMapper.h"
#include "Mappers/ISkillModelBuilder.h"
#include "Mappers/SkillDtoMapper.h"
#include "SkillComponent.generated.h"

class USkillSlot;
class USkillDataAsset;
class UGameplayAbility;
struct FSkillDomain;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillRegistered, int32 /* SlotIndex */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillUnregistered, int32 /* SlotIndex */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillsSwapped, int32 /* SlotIndexA */, int32 /* SlotIndexB */);
DECLARE_MULTICAST_DELEGATE(FOnSkillsChanged);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillStateChanged, const TArray<USkillSlot*>&);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SKILLMODULE_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USkillComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetMaxSlotCount() const { return MaxSkillSlots; }

	// ?�킬 ?�태가 변경될 ???�출?�는 ?�벤??(?�록, ?�거, ?�왑 ??
	// UPROPERTY(BlueprintAssignable, Category = "Skill|Events")
	FOnSkillStateChanged OnSkillStateChanged;

	// Domain Events (for DDD compliance)
	FOnSkillRegistered OnSkillRegistered;
	FOnSkillUnregistered OnSkillUnregistered;
	FOnSkillsSwapped OnSkillsSwapped;
	FOnSkillsChanged OnSkillsChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_SkillSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Component")
	TArray<TObjectPtr<USkillSlot>> SkillSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Component")
	int32 MaxSkillSlots = 8;

	// 3-Layer Mapping Architecture
	UPROPERTY()
	TScriptInterface<USkillDtoMapper> DtoMapper;

	UPROPERTY()
	TScriptInterface<ISkillAssetMapperInterface> AssetMapper;

	UPROPERTY()
	TScriptInterface<ISkillModelBuilderInterface> ModelBuilder;

public:
	// ========================================================================
	// AGGREGATE ROOT METHODS - BUSINESS LOGIC WITH INVARIANT PROTECTION
	// ========================================================================

	/**
	 * Register a skill to specific slot (Domain logic with validation)
	 * @param SlotIndex Target slot index
	 * @param SkillData Skill data to register
	 * @return True if successfully registered
	 */
	bool RegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData);

	/**
	 * Unregister a skill from slot (Domain logic with validation)
	 * @param SlotIndex Slot index to unregister
	 */
	void UnregisterSkill(int32 SlotIndex);

	/**
	 * Swap skills between two slots (Domain logic with validation)
	 * @param SlotIndexA First slot index
	 * @param SlotIndexB Second slot index
	 */
	void SwapSkills(int32 SlotIndexA, int32 SlotIndexB);

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
	 * @param SlotIndex Target slot index
	 * @param SkillData Skill data to validate
	 * @return True if skill can be registered
	 */
	bool CanRegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData) const;

	/**
	 * Validate if a skill can be unregistered (Domain Rule)
	 * @param SlotIndex Slot index to validate
	 * @return True if skill can be unregistered
	 */
	bool CanUnregisterSkill(int32 SlotIndex) const;

	/**
	 * Validate if skills can be swapped (Domain Rule)
	 * @param SlotIndexA First slot index
	 * @param SlotIndexB Second slot index
	 * @return True if skills can be swapped
	 */
	bool CanSwapSkills(int32 SlotIndexA, int32 SlotIndexB) const;

	/**
	 * Validate if cooldown can be updated (Domain Rule)
	 * @param SlotIndex Slot index to validate
	 * @param LastUsedTime The last used time
	 * @param RemainingCooldown The remaining cooldown
	 * @return True if cooldown can be updated
	 */
	bool CanUpdateCooldown(int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown) const;

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
	 * Get skill slot by index
	 * @param SlotIndex Slot index to find
	 * @return Found skill slot, nullptr if not found
	 */
	USkillSlot* GetSkillSlotByIndex(int32 SlotIndex) const;

	/**
	 * Get skill slot by slot key and index
	 * @param SlotKey Slot key to find
	 * @param SlotIndex Slot index to find
	 * @return Found skill slot, nullptr if not found
	 */
	USkillSlot* GetSkillSlotByKeyAndIndex(const FString& SlotKey, int32 SlotIndex) const;

	/**
	 * Get all skill slots (read-only access)
	 * @return Array of all skill slots
	 */
	const TArray<USkillSlot*>& GetAllSkillSlots() const { return reinterpret_cast<const TArray<USkillSlot*>&>(SkillSlots); }

	// ========================================================================
	// 3-LAYER MAPPING INTEGRATION METHODS
	// ========================================================================

	/**
	 * Load skill slots from database using 3-layer mapping
	 * @param UserId User ID to load skills for
	 * @param SlotKey Slot key (e.g., "ActionBar", "QuickSlot")
	 */
	void LoadSkillSlotsFromDatabase(int32 UserId, const FString& SlotKey);

	/**
	 * Save skill slots to database using 3-layer mapping
	 * @param UserId User ID to save skills for
	 */
	void SaveSkillSlotsToDatabase(int32 UserId);

	/**
	 * Build skill slots from DTOs and AssetData using mappers
	 * @param SlotDTOs Skill slot DTOs from database
	 * @param AssetDataArray Asset data from content
	 */
	void BuildSkillSlotsFromMappers(
		const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
		const TArray<USkillDataAsset*>& SkillDataAssets
	);

	/**
	 * Extract DTOs from current skill slots using mappers
	 * @param UserId User ID for the DTOs
	 * @return Array of skill slot DTOs
	 */
	TArray<FSkillSlotDatabaseDTO> ExtractDTOsFromSkillSlots(const FString& UserId) const;

	// ========================================================================
	// LEGACY DOMAIN INTEGRATION METHODS - DEPRECATED
	// ========================================================================

	/**
	 * DEPRECATED: Synchronize component state with domain data (called by DomainService)
	 * Use 3-Layer Mapping Architecture: BuildSkillSlotsFromMappers() instead
	 * Updates the aggregate's internal state to match the domain data
	 * @param SkillData Domain data to sync with
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: BuildSkillSlotsFromMappers() instead")
	void SyncWithDomain(const FSkillDomain& SkillData);

	/**
	 * DEPRECATED: Extract current domain data from component (called by DomainService)
	 * Use 3-Layer Mapping Architecture: ExtractDTOsFromSkillSlots() instead
	 * Creates domain data representation from current aggregate state
	 * @return Current skill domain data
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: ExtractDTOsFromSkillSlots() instead")
	FSkillDomain ExtractDomain() const;

	// ?��? 복제�??�행�?
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

	/**
	 * Initialize mappers (called in BeginPlay)
	 */
	void InitializeMappers();

	/**
	 * Validate mapper dependencies
	 * @return True if all mappers are valid
	 */
	bool ValidateMappers() const;
};