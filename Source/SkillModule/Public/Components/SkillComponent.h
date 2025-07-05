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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillStateChanged, const TArray<USkillSlot*>&, UpdatedSkills);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SKILLMODULE_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USkillComponent();

	int32 GetMaxSlotCount() const;

	// 스킬 상태가 변경될 때 호출되는 이벤트 (등록, 제거, 스왑 등)
	UPROPERTY(BlueprintAssignable, Category = "Skill|Events")
	FOnSkillStateChanged OnSkillStateChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_SkillSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Component")
	TArray<USkillSlot*> SkillSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Component")
	int32 MaxSkillSlots = 8;

public:
	// 슬롯 관리
	bool RegisterSkill(USkillDataAsset* SkillData);
	void UnregisterSkill(const FGuid& SlotId);
	void SwapSkills(const FGuid& SlotIdA, const FGuid& SlotIdB);
	USkillSlot* GetSkillSlotByGuid(const FGuid& SlotId) const;
	FGuid GetSkillSlotGuidByIndex(int32 index) const;
	const TArray<USkillSlot*>& GetAllSkillSlots() const { return SkillSlots; }

	// 도메인 검사 및 갱신

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnRep_SkillSlots();
}; 