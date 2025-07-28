
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillCastingService.generated.h"

struct FGuid;
class AActor;
class ISkillStateRepositoryInterface;
struct FSkillUsedEvent;

/**
 * ?�킬 ?�전 로직???�랜??�� ?�위�?처리?�는 ?�메???�비?�입?�다.
 */
UCLASS()
class SKILLMODULE_API USkillCastingService : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * ?�킬 ?�전???�도?�는 메인 ?�랜??�� 메서?�입?�다.
	 * @param Caster ?�킬???�전?�는 ?�터
	 * @param SlotIndex ?�전???�킬 ?�롯???�덱??
	 * @return ?�공?�으�??�전 과정???�작?�으�?true�?반환?�니??
	 */
	bool TryCastSkill(ACharacter* Caster, int32 SlotIndex);

private:
	// ?�제 구현?�서??Event Bus ?�스?�을 ?�해 ?�벤?��? 발행?�야 ?�니??
	// void PublishSkillUsedEvent(const FSkillUsedEvent& Event);
	
}; 