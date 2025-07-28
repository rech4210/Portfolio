
#pragma once

#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_MultiBoxTrace.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillTarget_MultiBoxTrace : public USkillTargetBase {
	GENERATED_BODY()
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
