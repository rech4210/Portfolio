#pragma once

#include "FSkillDataTypeContainer.generated.h"


UENUM(BlueprintType)
enum class ETargetDetectType : uint8 {
	Self,           // 자기 자신
	AreaAround,     // 주변 범위 감지 (근접)
	Directional,    // 방향 범위 (예: 전방 베기)
	TargetActor,    // 타겟 지정 (클릭 대상)
	GroundLocation,  // 지면 클릭 위치 기준
	autopilot      // 자동 조준 (예: 스킬 사용 시 자동으로 적을 조준)
};

USTRUCT(BlueprintType)
struct FSkillShapeConfig {
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	FVector BoxHalfExtent = FVector(100.f, 100.f, 100.f);

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float SphereRadius = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;
};