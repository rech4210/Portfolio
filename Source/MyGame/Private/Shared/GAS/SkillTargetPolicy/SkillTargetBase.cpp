
#include "Shared/GAS/SkillTargetPolicy/SkillTargetBase.h"

void USkillTargetBase::DebugSkillShape(const UWorld* World, const FVector& StartLocation, const FVector& EndLocation,
	const FSkillShapeConfig& Config) const {
#if WITH_EDITOR
	// 🔷 Box Trace 시각화
	DrawDebugBox(
		World,
		(StartLocation + EndLocation) * 0.5f, // 박스 중심
		Config.BoxHalfExtent,
		FQuat::Identity,
		FColor::Blue,
		false,
		1.0f,  // 지속 시간 (초)
		0,
		2.0f   // 선 두께
	);

	// 🟢 Sphere Overlap 시각화
	DrawDebugSphere(
		World,
		EndLocation,
		Config.SphereRadius,
		12,            // 세그먼트 수
		FColor::Green,
		false,
		1.0f,
		0,
		2.0f
	);

	// 🔴 Forward 방향 시각화
	DrawDebugLine(
		World,
		StartLocation,
		EndLocation,
		FColor::Red,
		false,
		1.0f,
		0,
		1.0f
	);
#endif
}
