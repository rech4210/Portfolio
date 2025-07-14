// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillCastingService.generated.h"

struct FGuid;
class AActor;
class ISkillStateRepositoryInterface;
struct FSkillUsedEvent;

/**
 * 스킬 시전 로직을 트랜잭션 단위로 처리하는 도메인 서비스입니다.
 */
UCLASS()
class SKILLMODULE_API USkillCastingService : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 스킬 시전을 시도하는 메인 트랜잭션 메서드입니다.
	 * @param Caster 스킬을 시전하는 액터
	 * @param SlotId 시전할 스킬 슬롯의 고유 ID
	 * @return 성공적으로 시전 과정을 시작했으면 true를 반환합니다.
	 */
	bool TryCastSkill(ACharacter* Caster, const FGuid& SlotId);

private:
	// 실제 구현에서는 Event Bus 시스템을 통해 이벤트를 발행해야 합니다.
	// void PublishSkillUsedEvent(const FSkillUsedEvent& Event);
	
}; 