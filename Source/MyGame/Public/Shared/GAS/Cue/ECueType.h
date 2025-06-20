#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ECueType : uint8{
    None,
    DirectionPreview,
    AreaAttackWarning
};