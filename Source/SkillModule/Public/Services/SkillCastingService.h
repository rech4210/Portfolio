// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillCastingService.generated.h"

struct FGuid;
class AActor;
class ISkillStateRepositoryInterface;
struct FSkillUsedEvent;

/**
 * ?¤í‚¬ ?œì „ ë¡œì§???¸ëœ??…˜ ?¨ìœ„ë¡?ì²˜ë¦¬?˜ëŠ” ?„ë©”???œë¹„?¤ì…?ˆë‹¤.
 */
UCLASS()
class SKILLMODULE_API USkillCastingService : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * ?¤í‚¬ ?œì „???œë„?˜ëŠ” ë©”ì¸ ?¸ëœ??…˜ ë©”ì„œ?œì…?ˆë‹¤.
	 * @param Caster ?¤í‚¬???œì „?˜ëŠ” ?¡í„°
	 * @param SlotIndex ?œì „???¤í‚¬ ?¬ë¡¯???¸ë±??
	 * @return ?±ê³µ?ìœ¼ë¡??œì „ ê³¼ì •???œì‘?ˆìœ¼ë©?trueë¥?ë°˜í™˜?©ë‹ˆ??
	 */
	bool TryCastSkill(ACharacter* Caster, int32 SlotIndex);

private:
	// ?¤ì œ êµ¬í˜„?ì„œ??Event Bus ?œìŠ¤?œì„ ?µí•´ ?´ë²¤?¸ë? ë°œí–‰?´ì•¼ ?©ë‹ˆ??
	// void PublishSkillUsedEvent(const FSkillUsedEvent& Event);
	
}; 