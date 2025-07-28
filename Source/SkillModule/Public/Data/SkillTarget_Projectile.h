#pragma once
#include "CoreMinimal.h"
#include "SkillTargetBase.h"
#include "SkillTarget_Projectile.generated.h"

/**
 * 
 */
UCLASS()
class SKILLMODULE_API USkillTarget_Projectile : public USkillTargetBase {
	GENERATED_BODY()
	virtual TArray<AActor*> DetectTargets(FSkillContext& SkillContext) override;
};
