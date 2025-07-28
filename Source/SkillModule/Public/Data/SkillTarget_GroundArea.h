
#pragma once

#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_GroundArea.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillTarget_GroundArea : public USkillTargetBase {
	GENERATED_BODY()
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
