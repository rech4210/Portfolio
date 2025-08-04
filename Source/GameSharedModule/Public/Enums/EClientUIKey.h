#pragma once

#include "CoreMinimal.h"
#include "EClientUIKey.generated.h"

UENUM(BlueprintType)
enum class EClientUIKey : uint8
{
	None = 0,
	
	AuthComponent,
	LoginUI,
	HUD,
	
	MAX UMETA(Hidden)
};

struct GAMESHAREDMODULE_API FClientUIKeyUtils
{
	static FString ToString(EClientUIKey Key);

	static EClientUIKey FromString(const FString& KeyString);

	static bool IsValidKey(EClientUIKey Key);
};
