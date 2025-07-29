#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SkillDataAsset.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Mappers/ISkillDtoMapper.h"
#include "Mappers/ISkillAssetMapper.h"
#include "Mappers/ISkillModelBuilder.h"
#include "Mappers/SkillDtoMapper.h"
#include "Engine/NetSerialization.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameSharedModule/Public/Interface/IClientComponentProvider.h"
#include "SkillComponent.generated.h"

class USkillSlot;
class USkillDataAsset;
class UGameplayAbility;
struct FSkillDomain;

// ============================================================================
// UI 업데이트용 스킬 슬롯 데이터 구조체
// ============================================================================

/**
 * 완전한 스킬 슬롯 데이터 구조체 (USkillSlot의 완전한 대체)
 * 네트워크 복제와 로컬 사용 모두 지원
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillSlotReplicationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 SkillId = 0;

	UPROPERTY(BlueprintReadOnly)
	FString SlotKey;

	UPROPERTY(BlueprintReadOnly)
	FDateTime LastUsedTime;

	// SkillDataAsset 직접 포함 (복제 지원)
	UPROPERTY(BlueprintReadOnly)
	USkillDataAsset* SkillData = nullptr;

	// UI에 필요한 기본 스킬 정보 (캐시된 데이터)
	UPROPERTY(BlueprintReadOnly)
	FString SkillName;

	UPROPERTY(BlueprintReadOnly)
	FString SkillDescription;

	UPROPERTY(BlueprintReadOnly)
	float Cooldown = 0.0f;

	FSkillSlotReplicationData()
	{
		SlotIndex = -1;
		SkillId = 0;
		LastUsedTime = FDateTime::MinValue();
		Cooldown = 0.0f;
		SkillData = nullptr;
	}

	// SkillSlot의 기존 기능들을 직접 제공
	bool IsEmpty() const { return SkillId <= 0 || !SkillData; }
	
	bool IsOnCooldown(float BaseCooltime) const
	{
		if (BaseCooltime <= 0.0f || LastUsedTime <= FDateTime::MinValue())
		{
			return false;
		}
		FDateTime Now = FDateTime::Now();
		double ElapsedSeconds = (Now - LastUsedTime).GetTotalSeconds();
		return ElapsedSeconds < BaseCooltime;
	}
	
	float GetRemainingCooldown(float BaseCooltime) const
	{
		if (BaseCooltime <= 0.0f || LastUsedTime <= FDateTime::MinValue())
		{
			return 0.0f;
		}
		FDateTime Now = FDateTime::Now();
		double ElapsedSeconds = (Now - LastUsedTime).GetTotalSeconds();
		return FMath::Max(0.0f, BaseCooltime - static_cast<float>(ElapsedSeconds));
	}

	// 스킬 데이터 설정
	void SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId)
	{
		SkillData = InSkillData;
		SkillId = InSkillId;
		LastUsedTime = FDateTime::MinValue();
		
		// 캐시된 데이터 업데이트
		if (InSkillData)
		{
			SkillName = InSkillData->DisplayName.ToString();
			SkillDescription = InSkillData->Description.ToString();
			Cooldown = InSkillData->CoolTime;
		}
		else
		{
			SkillName.Empty();
			SkillDescription.Empty();
			Cooldown = 0.0f;
		}
	}

	// 스킬 클리어
	void ClearSkill()
	{
		SkillData = nullptr;
		SkillId = 0;
		LastUsedTime = FDateTime::MinValue();
		SkillName.Empty();
		SkillDescription.Empty();
		Cooldown = 0.0f;
	}

	// 초기화
	void Initialize(int32 InSlotIndex, const FString& InSlotKey, USkillDataAsset* InSkillData = nullptr)
	{
		SlotIndex = InSlotIndex;
		SlotKey = InSlotKey;
		if (InSkillData)
		{
			SetSkillData(InSkillData, InSkillData->SkillID);
		}
		else
		{
			ClearSkill();
		}
	}
};

/**
 * FastArraySerializer 아이템
 */
USTRUCT()
struct SKILLMODULE_API FSkillSlotReplicationItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FSkillSlotReplicationData SlotData;

	FSkillSlotReplicationItem()
	{
	}

	FSkillSlotReplicationItem(const FSkillSlotReplicationData& InSlotData)
		: SlotData(InSlotData)
	{
	}
};

/**
 * FastArraySerializer를 상속한 스킬 슬롯 배열 컨테이너
 */
USTRUCT()
struct SKILLMODULE_API FSkillSlotReplicationArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSkillSlotReplicationItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSkillSlotReplicationItem, FSkillSlotReplicationArray>(Items, DeltaParms, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);

	class USkillComponent* GetOwner() const { return Owner; }
	void SetOwner(class USkillComponent* InOwner) { Owner = InOwner; }

private:
	UPROPERTY()
	class USkillComponent* Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FSkillSlotReplicationArray> : public TStructOpsTypeTraitsBase2<FSkillSlotReplicationArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

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

	// UPROPERTY(BlueprintAssignable, Category = "Skill|Events")
	FOnSkillStateChanged OnSkillStateChanged;

	// Domain Events (for DDD compliance)
	FOnSkillRegistered OnSkillRegistered;
	FOnSkillUnregistered OnSkillUnregistered;
	FOnSkillsSwapped OnSkillsSwapped;
	FOnSkillsChanged OnSkillsChanged;

protected:
	// 주 데이터 저장소 - FSkillSlotReplicationData 배열 (복제 및 로컬 모두 지원)
	UPROPERTY(ReplicatedUsing=OnRep_SkillSlotsReplication)
	FSkillSlotReplicationArray SkillSlotsReplication;

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
	 * Check if player has a specific skill
	 * @param SkillData Skill data to check
	 * @return True if player has this skill
	 */
	bool HasSkill(USkillDataAsset* SkillData) const;

	// ========================================================================
	// QUERY METHODS - READ-ONLY ACCESS
	// ========================================================================

	/**
	 * Get skill slot replication data by index (primary access method)
	 * @param SlotIndex Slot index to find
	 * @return Skill slot replication data, nullptr if not found
	 */
	const FSkillSlotReplicationData* GetSkillSlotDataByIndex(int32 SlotIndex) const;

	/**
	 * Get mutable skill slot data by index (for modifications)
	 * @param SlotIndex Slot index to find
	 * @return Mutable skill slot replication data, nullptr if not found
	 */
	FSkillSlotReplicationData* GetMutableSkillSlotDataByIndex(int32 SlotIndex);


	/**
	 * Get skill slot by slot key and index
	 * @param SlotKey Slot key to find
	 * @param SlotIndex Slot index to find
	 * @return Skill slot data, nullptr if not found
	 */
	const FSkillSlotReplicationData* GetSkillSlotDataByKeyAndIndex(const FString& SlotKey, int32 SlotIndex) const;

	/**
	 * Get all skill slot data (read-only access)
	 * @return Array of all skill slot data
	 */
	TArray<FSkillSlotReplicationData> GetAllSkillSlotsData() const;


	/**
	 * Get replicated skill slots data for UI (read-only access)
	 * @return FastArray containing replicated skill slot data
	 */
	const FSkillSlotReplicationArray& GetReplicatedSkillSlots() const { return SkillSlotsReplication; }

	// ========================================================================
	// 3-LAYER MAPPING INTEGRATION METHODS
	// ========================================================================

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

	// ?��? 복제�??�행�?
	// virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_SkillSlotsReplication();

public:
	// ========================================================================
	// SKILL REPLICATION METHODS
	// ========================================================================

	/**
	 * 스킬 슬롯 변경사항을 복제 시스템에 반영
	 * @param SlotIndex 변경된 슬롯 인덱스
	 */
	void MarkSlotForReplication(int32 SlotIndex);

	/**
	 * 모든 스킬 슬롯을 복제 시스템에 동기화
	 */
	void SyncAllSlotsToReplication();

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

	/**
	 * Initialize empty skill slots (server only)
	 */
	void InitializeEmptySlots();
};