
#pragma once

#include "CoreMinimal.h"
#include "SkillSlot.generated.h"

class USkillDataAsset;
class UGameplayAbility;

UCLASS(BlueprintType, Blueprintable)
class SKILLMODULE_API USkillSlot : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SlotIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SkillId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	TObjectPtr<USkillDataAsset> SkillData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FDateTime LastUsedTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FString SlotKey;

	void Initialize(int32 InSlotIndex, const FString& InSlotKey, USkillDataAsset* InSkillData = nullptr);
	void SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId);
	void ClearSkill();
	
	bool IsEmpty() const { return SkillData == nullptr || SkillId <= 0; }
	bool IsOnCooldown(float BaseCooltime) const;
	float GetRemainingCooldown(float BaseCooltime) const;
	
}; 