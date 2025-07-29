
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "SkillCastingService.generated.h"

struct FGuid;
class AActor;
class ISkillStateRepositoryInterface;
struct FSkillUsedEvent;

UCLASS()
class SKILLMODULE_API USkillCastingService : public UActorComponent
{
	GENERATED_BODY()

public:

	bool TryCastSkill(ACharacter* Caster, int32 SlotIndex);

private:
}; 