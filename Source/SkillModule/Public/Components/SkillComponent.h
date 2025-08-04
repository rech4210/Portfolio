#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SkillDataAsset.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Mappers/ISkillAssetMapper.h"
#include "Mappers/ISkillModelBuilder.h"
#include "Mappers/SkillDtoMapper.h"
#include "Engine/NetSerialization.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "SkillComponent.generated.h"

class USkillSlot;
class USkillDataAsset;
class UGameplayAbility;
struct FSkillDomain;

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

	UPROPERTY(BlueprintReadOnly)
	USkillDataAsset* SkillData = nullptr;

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

	void SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId)
	{
		SkillData = InSkillData;
		SkillId = InSkillId;
		LastUsedTime = FDateTime::MinValue();
		
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

	void ClearSkill()
	{
		SkillData = nullptr;
		SkillId = 0;
		LastUsedTime = FDateTime::MinValue();
		SkillName.Empty();
		SkillDescription.Empty();
		Cooldown = 0.0f;
	}

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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillRegistered, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillUnregistered, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillsSwapped, int32, int32);
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

	FOnSkillStateChanged OnSkillStateChanged;

	FOnSkillRegistered OnSkillRegistered;
	FOnSkillUnregistered OnSkillUnregistered;
	FOnSkillsSwapped OnSkillsSwapped;
	FOnSkillsChanged OnSkillsChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_SkillSlotsReplication)
	FSkillSlotReplicationArray SkillSlotsReplication;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Component")
	int32 MaxSkillSlots = 8;

	UPROPERTY()
	TScriptInterface<USkillDtoMapper> DtoMapper;

	UPROPERTY()
	TScriptInterface<ISkillAssetMapperInterface> AssetMapper;

	UPROPERTY()
	TScriptInterface<ISkillModelBuilderInterface> ModelBuilder;

public:
	bool RegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData);

	void UnregisterSkill(int32 SlotIndex);

	void SwapSkills(int32 SlotIndexA, int32 SlotIndexB);

	bool CanRegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData) const;

	bool CanUnregisterSkill(int32 SlotIndex) const;

	bool CanSwapSkills(int32 SlotIndexA, int32 SlotIndexB) const;

	bool HasSkill(USkillDataAsset* SkillData) const;

	const FSkillSlotReplicationData* GetSkillSlotDataByIndex(int32 SlotIndex) const;

	FSkillSlotReplicationData* GetMutableSkillSlotDataByIndex(int32 SlotIndex);

	const FSkillSlotReplicationData* GetSkillSlotDataByKeyAndIndex(const FString& SlotKey, int32 SlotIndex) const;

	TArray<FSkillSlotReplicationData> GetAllSkillSlotsData() const;

	const FSkillSlotReplicationArray& GetReplicatedSkillSlots() const { return SkillSlotsReplication; }

	void BuildSkillSlotsFromMappers(
		const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
		const TArray<USkillDataAsset*>& SkillDataAssets
	);

	TArray<FSkillSlotDatabaseDTO> ExtractDTOsFromSkillSlots(const FString& UserId) const;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_SkillSlotsReplication();

public:
	void MarkSlotForReplication(int32 SlotIndex);

	void SyncAllSlotsToReplication();

private:
	void NotifySkillStateChanged();

	void InitializeMappers();

	bool ValidateMappers() const;

	void InitializeEmptySlots();
};