// @Needmodifi
#pragma once
#include "Engine/EngineTypes.h"
#include "FSkillShapeConfig.generated.h"

USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillShapeConfig {
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