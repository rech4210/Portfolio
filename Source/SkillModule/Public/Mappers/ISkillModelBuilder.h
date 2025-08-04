#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Mappers/ISkillAssetMapper.h"
#include "ISkillModelBuilder.generated.h"

struct FSkillSlotReplicationData;
class USkillDataAsset;

USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillDomainModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 UserId = 0;

	UPROPERTY(BlueprintReadOnly)
	FString SlotKey;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 SkillId = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SkillLevel = 1;

	UPROPERTY(BlueprintReadOnly)
	FDateTime LastUsedTime;

	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;

	// Business Logic Properties
	UPROPERTY(BlueprintReadOnly)
	float RemainingCooldown = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bCanUse = true;

	UPROPERTY(BlueprintReadOnly)
	USkillDataAsset* SkillDataAsset = nullptr;

	FSkillDomainModel()
	{
		UserId = 0;
		SlotKey = TEXT("");
		SlotIndex = -1;
		SkillId = 0;
		SkillLevel = 1;
		LastUsedTime = FDateTime::MinValue();
		bIsActive = false;
		RemainingCooldown = 0.0f;
		bCanUse = true;
	}
};

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class SKILLMODULE_API USkillModelBuilderInterface : public UInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillModelBuilderInterface
{
	GENERATED_BODY()

public:
	virtual FSkillDomainModel BuildDomainModel(
		const FSkillSlotDatabaseDTO& SlotDTO, 
		USkillDataAsset* SkillDataAsset
	) = 0;

	virtual FSkillSlotReplicationData BuildSkillSlotData(const FSkillDomainModel& DomainModel) = 0;

	virtual float CalculateRemainingCooldown(
		const FDateTime& LastUsedTime, 
		float BaseCooldown
	) = 0;

	virtual bool CanUseSkill(
		const FDateTime& LastUsedTime, 
		float BaseCooldown,
		int32 CurrentMana,
		int32 RequiredMana
	) = 0;

	virtual int32 CalculateScaledValue(int32 BaseValue, int32 SkillLevel) = 0;
	virtual float CalculateScaledValue(float BaseValue, int32 SkillLevel) = 0;

	virtual FSkillSlotDatabaseDTO ExtractSlotDTO(const FSkillSlotReplicationData& SlotData) = 0;
	virtual FSkillDomainModel ExtractDomainModel(const FSkillSlotReplicationData& SlotData) = 0;

	virtual TArray<FSkillDomainModel> BuildDomainModels(
		const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
		const TArray<USkillDataAsset*>& SkillDataAssets
	) = 0;

	virtual TArray<FSkillSlotReplicationData> BuildSkillSlotDataArray(
		const TArray<FSkillDomainModel>& DomainModels
	) = 0;

	virtual bool ValidateDomainModel(const FSkillDomainModel& Model, FString& OutErrorMessage) = 0;
	virtual bool ValidateSkillSlotData(const FSkillSlotReplicationData& SlotData, FString& OutErrorMessage) = 0;
};
