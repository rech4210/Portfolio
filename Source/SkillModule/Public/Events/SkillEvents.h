
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SkillEvents.generated.h"

/**
 * ?�킬???�공?�으�??�용?�었????발생?�는 ?�메???�벤?�입?�다.
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
 * ?�킬??쿨다?�이 종료?�었????발생?�는 ?�메???�벤?�입?�다.
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