// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillSlot.generated.h"

class USkillDataAsset;
class UGameplayAbility;
/**
 * 스킬 슬롯의 상태를 나타내는 Entity
 * 각 슬롯은 고유한 ID와 마지막 사용 시간 등의 상태를 가집니다.
 */
UCLASS(BlueprintType, Blueprintable)
class SKILLMODULE_API USkillSlot : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	// 고유 식별자
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FGuid SlotId;

	// 스킬 데이터 (Value Object 참조)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	TObjectPtr<USkillDataAsset> SkillData;

	// 슬롯에 할당된 GameplayAbility 클래스
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	TSubclassOf<UGameplayAbility> AbilityClass;

	// 마지막 사용 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Entity")
	FDateTime LastUsedTime;

	void Initialize(USkillDataAsset* InSkillData, TSubclassOf<UGameplayAbility> InAbilityClass);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
}; 