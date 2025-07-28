#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "SkillTargetActor_Mouse.generated.h"

UCLASS()
class SKILLMODULE_API ASkillTargetActor_Mouse : public AGameplayAbilityTargetActor {
	GENERATED_BODY()

public:
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
protected:
	UPROPERTY()
	APlayerController* PlayerController;
};

