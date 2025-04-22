#pragma once

#include "CoreMinimal.h"
#include "../AbilityInputID.h"
#include "GA_Base.h"
#include "GA_Skill3.generated.h"

class UBuffDataAsset;
class USkillDataAsset;
class UPrimaryDataAsset;

UCLASS()
class MYGAME_API UGA_Skill3 : public UGA_Base
{
	GENERATED_BODY()

public:
	UGA_Skill3();

	UPROPERTY()
	EAbilityInputID AbilityInputID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UBuffDataAsset> BuffDataAsset;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};
