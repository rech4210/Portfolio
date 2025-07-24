// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillSlot.generated.h"

class USkillDataAsset;
class UGameplayAbility;
/**
 * 스킬 슬롯의 상태를 나타내는 Entity
 * SlotIndex 기반으로 식별하며, SQL 스키마와 일치합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class SKILLMODULE_API USkillSlot : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	// 슬롯 인덱스 (0, 1, 2, 3... 순서 기반 식별자)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SlotIndex;

	// 스킬 ID (데이터베이스의 skill_id)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	int32 SkillId;

	// 스킬 데이터 (Value Object 참조)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	TObjectPtr<USkillDataAsset> SkillData;

	// 마지막 사용 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FDateTime LastUsedTime;

	// 슬롯 키 (Q, W, E, R 등)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FString SlotKey;

	void Initialize(int32 InSlotIndex, const FString& InSlotKey, USkillDataAsset* InSkillData = nullptr);
	void SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId);
	void ClearSkill();
	
	// 비즈니스 로직
	bool IsEmpty() const { return SkillData == nullptr || SkillId <= 0; }
	bool IsOnCooldown(float BaseCooltime) const;
	float GetRemainingCooldown(float BaseCooltime) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
}; 