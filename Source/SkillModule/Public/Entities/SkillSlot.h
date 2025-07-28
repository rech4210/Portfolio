
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillSlot.generated.h"

class USkillDataAsset;
class UGameplayAbility;
/**
 * ?�킬 ?�롯???�태�??��??�는 Entity
 * SlotIndex 기반?�로 ?�별?�며, SQL ?�키마�? ?�치?�니??
 */
UCLASS(BlueprintType, Blueprintable)
class SKILLMODULE_API USkillSlot : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	// ?�롯 ?�덱??(0, 1, 2, 3... ?�서 기반 ?�별??
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SlotIndex;

	// ?�킬 ID (?�이?�베?�스??skill_id)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SkillId;

	// ?�킬 ?�이??(Value Object 참조)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	TObjectPtr<USkillDataAsset> SkillData;

	// 마�?�??�용 ?�간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FDateTime LastUsedTime;

	// ?�롯 ??(Q, W, E, R ??
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FString SlotKey;

	void Initialize(int32 InSlotIndex, const FString& InSlotKey, USkillDataAsset* InSkillData = nullptr);
	void SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId);
	void ClearSkill();
	
	// 비즈?�스 로직
	bool IsEmpty() const { return SkillData == nullptr || SkillId <= 0; }
	bool IsOnCooldown(float BaseCooltime) const;
	float GetRemainingCooldown(float BaseCooltime) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
}; 