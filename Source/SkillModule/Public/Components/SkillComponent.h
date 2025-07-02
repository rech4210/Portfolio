// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

class USkillSlot;
class USkillDataAsset;
class UGameplayAbility;

/**
 * 캐릭터의 스킬 슬롯들을 관리하는 Aggregate Root 역할을 하는 컴포넌트입니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKILLMODULE_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Component")
	TArray<TObjectPtr<USkillSlot>> SkillSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Component")
	int32 MaxSkillSlots = 8;

public:
	// 슬롯 관리
	bool RegisterSkill(const USkillDataAsset* SkillData, TSubclassOf<UGameplayAbility> AbilityClass);
	void UnregisterSkill(const FGuid& SlotId);
	void SwapSkills(const FGuid& SlotIdA, const FGuid& SlotIdB);
	USkillSlot* GetSkillSlot(const FGuid& SlotId) const;
	FGuid GetSkillSlotIDByIndex(int32 index) const;
	const TArray<TObjectPtr<USkillSlot>>& GetAllSkillSlots() const { return SkillSlots; }

	// 도메인 검사 및 갱신

protected:
	virtual void BeginPlay() override;
}; 