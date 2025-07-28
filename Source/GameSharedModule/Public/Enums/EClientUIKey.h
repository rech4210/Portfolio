#pragma once

#include "CoreMinimal.h"
#include "EClientUIKey.generated.h"

/**
 * 클라이언트 UI 컴포넌트 식별 키
 * 서버/클라이언트 공통으로 사용 가능한 enum
 */
UENUM(BlueprintType)
enum class EClientUIKey : uint8
{
	None = 0,
	
	// 인증 관련 UI
	AuthComponent,
	LoginUI,
	
	MAX UMETA(Hidden)
};

/**
 * EClientUIKey 유틸리티 함수들
 */
struct GAMESHAREDMODULE_API FClientUIKeyUtils
{

	static FString ToString(EClientUIKey Key);

	static EClientUIKey FromString(const FString& KeyString);

	static bool IsValidKey(EClientUIKey Key);
};
