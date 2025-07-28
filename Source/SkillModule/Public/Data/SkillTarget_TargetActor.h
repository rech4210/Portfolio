
#pragma once

#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_TargetActor.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillTarget_TargetActor : public USkillTargetBase {
	GENERATED_BODY()
public:
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
