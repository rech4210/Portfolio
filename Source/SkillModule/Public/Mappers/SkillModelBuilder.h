#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Mappers/ISkillModelBuilder.h"
#include "SkillModelBuilder.generated.h"

// Forward declarations
class USkillSlot;

/**
 * DataAsset ??DomainModel 빌딩 구현�?
 * DataAsset�?비즈?�스 로직??결합?�여 ?�전???�메??모델 ?�성
 */
UCLASS(BlueprintType)
class SKILLMODULE_API USkillModelBuilder : public UObject, public ISkillModelBuilderInterface
{
	GENERATED_BODY()

public:
	USkillModelBuilder();

	// ISkillModelBuilderInterface interface
	virtual FSkillDomainModel BuildDomainModel(
		const FSkillSlotDatabaseDTO& SlotDTO, 
		USkillDataAsset* SkillDataAsset
	) override;

	virtual USkillSlot* BuildSkillSlotEntity(const FSkillDomainModel& DomainModel) override;

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

	virtual FSkillSlotDatabaseDTO ExtractSlotDTO(const USkillSlot* SkillSlot) override;
	virtual FSkillDomainModel ExtractDomainModel(const USkillSlot* SkillSlot) override;

	virtual TArray<FSkillDomainModel> BuildDomainModels(
		const TArray<FSkillSlotDatabaseDTO>& SlotDTOs,
		const TArray<USkillDataAsset*>& SkillDataAssets
	) override;

	virtual TArray<USkillSlot*> BuildSkillSlotEntities(
		const TArray<FSkillDomainModel>& DomainModels
	) override;

	virtual bool ValidateDomainModel(const FSkillDomainModel& Model, FString& OutErrorMessage) override;
	virtual bool ValidateSkillSlotEntity(const USkillSlot* SkillSlot, FString& OutErrorMessage) override;

protected:
	// ?�킬 ?�벨�??��??�링 계수
	UPROPERTY(EditDefaultsOnly, Category = "Skill Scaling")
	float SkillLevelScalingFactor = 1.2f;

	// 기본 값들
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float DefaultCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	int32 DefaultManaCost = 10;

private:
	// Helper functions
	bool IsValidDateTime(const FDateTime& DateTime);
};
