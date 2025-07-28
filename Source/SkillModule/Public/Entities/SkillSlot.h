// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillSlot.generated.h"

class USkillDataAsset;
class UGameplayAbility;
/**
 * ?§ÌÇ¨ ?¨Î°Ø???ÅÌÉúÎ•??òÌ??¥Îäî Entity
 * SlotIndex Í∏∞Î∞ò?ºÎ°ú ?ùÎ≥Ñ?òÎ©∞, SQL ?§ÌÇ§ÎßàÏ? ?ºÏπò?©Îãà??
 */
UCLASS(BlueprintType, Blueprintable)
class SKILLMODULE_API USkillSlot : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	// ?¨Î°Ø ?∏Îç±??(0, 1, 2, 3... ?úÏÑú Í∏∞Î∞ò ?ùÎ≥Ñ??
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SlotIndex;

	// ?§ÌÇ¨ ID (?∞Ïù¥?∞Î≤†?¥Ïä§??skill_id)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SkillId;

	// ?§ÌÇ¨ ?∞Ïù¥??(Value Object Ï∞∏Ï°∞)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	TObjectPtr<USkillDataAsset> SkillData;

	// ÎßàÏ?Îß??¨Ïö© ?úÍ∞Ñ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FDateTime LastUsedTime;

	// ?¨Î°Ø ??(Q, W, E, R ??
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FString SlotKey;

	void Initialize(int32 InSlotIndex, const FString& InSlotKey, USkillDataAsset* InSkillData = nullptr);
	void SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId);
	void ClearSkill();
	
	// ÎπÑÏ¶à?àÏä§ Î°úÏßÅ
	bool IsEmpty() const { return SkillData == nullptr || SkillId <= 0; }
	bool IsOnCooldown(float BaseCooltime) const;
	float GetRemainingCooldown(float BaseCooltime) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
}; 