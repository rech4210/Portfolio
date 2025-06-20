#pragma once

#include "CoreMinimal.h"
#include "EPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8{
	Idle,
	Combat,
	Stunned,
	Knockback,
	Dead,
	Hit
};
