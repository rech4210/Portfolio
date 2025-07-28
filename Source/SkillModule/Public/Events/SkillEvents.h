// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SkillEvents.generated.h"

/**
 * ?¤í‚¬???±ê³µ?ìœ¼ë¡??¬ìš©?˜ì—ˆ????ë°œìƒ?˜ëŠ” ?„ë©”???´ë²¤?¸ì…?ˆë‹¤.
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
 * ?¤í‚¬??ì¿¨ë‹¤?´ì´ ì¢…ë£Œ?˜ì—ˆ????ë°œìƒ?˜ëŠ” ?„ë©”???´ë²¤?¸ì…?ˆë‹¤.
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