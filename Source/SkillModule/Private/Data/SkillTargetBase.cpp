#include "Data/SkillTargetBase.h"
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"

void USkillTargetBase::DebugSkillShape(const UWorld* World, const FVector& StartLocation, const FVector& EndLocation,
									   const FSkillShapeConfig& Config) const {
#if WITH_EDITOR
	DrawDebugBox(
		World,
		(StartLocation + EndLocation) * 0.5f,
		Config.BoxHalfExtent,
		FQuat::Identity,
		FColor::Blue,
		false,
		2.0f, 
		0,
		2.0f 
	);

	DrawDebugSphere(
		World,
		EndLocation,
		Config.SphereRadius,
		12,
		FColor::Green,
		false,
		2.0f,
		0,
		2.0f
	);

	DrawDebugLine(
		World,
		StartLocation,
		EndLocation,
		FColor::Red,
		false,
		2.0f,
		0,
		1.0f
	);
#endif
}

bool USkillTargetBase::HasASC(AActor* Actor) const {
	if (Actor->Implements<UAbilitySystemInterface>()) {
		return true;
	}
	return false;
}
