#include "Shared/GAS/SkillTargetPolicy/FSkillContext.h"

FSkillContext::FSkillContext(
	UAbilitySystemComponent* InSourceASC,
	AActor* InSourceActor,
	AActor* InTargetActor = nullptr,
	const FVector& InTargetLocation = FVector::Zero(),
	USkillDataAsset* InSkillData = nullptr,
	int32 InComboIndex,
	float InStartTime)
	: SourceASC(InSourceASC)
	, SourceActor(InSourceActor)
	, TargetActor(InTargetActor)
	, TargetLocation(InTargetLocation)
	, SkillData(InSkillData)
	, ComboIndex(InComboIndex)
	, StartTime(InStartTime)
{}
