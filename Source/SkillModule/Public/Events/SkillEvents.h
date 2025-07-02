// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SkillEvents.generated.h"

/**
 * 스킬이 성공적으로 사용되었을 때 발생하는 도메인 이벤트입니다.
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillUsedEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Skill|Event")
	FGuid SlotId;

	UPROPERTY(BlueprintReadOnly, Category="Skill|Event")
	FDateTime Timestamp;

	FSkillUsedEvent() : SlotId(FGuid()), Timestamp(0) {}
	FSkillUsedEvent(const FGuid& InSlotId, const FDateTime& InTimestamp)
		: SlotId(InSlotId), Timestamp(InTimestamp) {}
};

/**
 * 스킬의 쿨다운이 종료되었을 때 발생하는 도메인 이벤트입니다.
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FCooldownExpiredEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Skill|Event")
	FGuid SlotId;
    
	FCooldownExpiredEvent() : SlotId(FGuid()) {}
	FCooldownExpiredEvent(const FGuid& InSlotId) : SlotId(InSlotId) {}
}; 