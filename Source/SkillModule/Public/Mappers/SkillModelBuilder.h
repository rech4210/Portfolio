#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Mappers/ISkillModelBuilder.h"
#include "SkillModelBuilder.generated.h"

struct FSkillSlotReplicationData;

UCLASS(BlueprintType)
class SKILLMODULE_API USkillModelBuilder : public UObject, public ISkillModelBuilderInterface
{
	GENERATED_BODY()

public:
	USkillModelBuilder();

	virtual FSkillDomainModel BuildDomainModel(
		const FSkillSlotDatabaseDTO& SlotDTO, 
		USkillDataAsset* SkillDataAsset
	) override;

	virtual FSkillSlotReplicationData BuildSkillSlotData(const FSkillDomainModel& DomainModel) override;

	virtual float CalculateRemainingCooldown(
		const FDateTime& LastUsedTime, 
		float BaseCooldown
	) override;

	virtual bool CanUseSkill(
		const FDateTime& LastUsedTime, 
		float BaseCooldown,
		int32 CurrentMana,
		int32 RequiredMana
	) override;

	virtual int32 CalculateScaledValue(int32 BaseValue, int32 SkillLevel) override;
	virtual float CalculateScaledValue(float BaseValue, int32 SkillLevel) override;

	virtual FSkillSlotDatabaseDTO ExtractSlotDTO(const FSkillSlotReplicationData& SlotData) override;
	virtual FSkillDomainModel ExtractDomainModel(const FSkillSlotReplicationData& SlotData) override;

	virtual TArray<FSkillDomainModel> BuildDomainModels(
		const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
		const TArray<USkillDataAsset*>& SkillDataAssets
	) override;

	virtual TArray<FSkillSlotReplicationData> BuildSkillSlotDataArray(
		const TArray<FSkillDomainModel>& DomainModels
	) override;

	virtual bool ValidateDomainModel(const FSkillDomainModel& Model, FString& OutErrorMessage) override;
	virtual bool ValidateSkillSlotData(const FSkillSlotReplicationData& SlotData, FString& OutErrorMessage) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill Scaling")
	float SkillLevelScalingFactor = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float DefaultCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	int32 DefaultManaCost = 10;

private:
	bool IsValidDateTime(const FDateTime& DateTime);
	bool IsOnCooldown(const FDateTime& LastUsedTime, float BaseCooltime) const;
};
