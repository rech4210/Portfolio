// @Needmodifi
#pragma once

#include "EGasDataType.generated.h"

UENUM(BlueprintType)
enum class EGasDataType : uint8
{
	None,
	Cooldown,
	CueDuration,
	SkillID,
	TagDuration
}; 