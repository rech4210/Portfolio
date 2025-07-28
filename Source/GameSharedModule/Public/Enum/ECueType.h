#pragma once

#include "ECueType.generated.h"

UENUM(BlueprintType)
enum class ECueType : uint8 {
	None,
	DirectionPreview,
	AreaAttackWarning
}; 