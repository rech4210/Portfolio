#pragma once

#include "CoreMinimal.h"

/**
 * 클라이언트 UI 시스템 디버깅 로그 매크로
 */
DECLARE_LOG_CATEGORY_EXTERN(LogClientUI, Log, All);

#define CLIENT_UI_LOG(Verbosity, Format, ...) \
	UE_LOG(LogClientUI, Verbosity, TEXT("[CLIENT_UI] ") Format, ##__VA_ARGS__)

#define CLIENT_UI_LOG_FUNC(Verbosity, Format, ...) \
	UE_LOG(LogClientUI, Verbosity, TEXT("[CLIENT_UI] %s: ") Format, *FString(__FUNCTION__), ##__VA_ARGS__)

// 조건부 로깅 (클라이언트 전용)
#if !UE_SERVER
	#define CLIENT_UI_LOG_CLIENT(Verbosity, Format, ...) CLIENT_UI_LOG(Verbosity, Format, ##__VA_ARGS__)
#else
	#define CLIENT_UI_LOG_CLIENT(Verbosity, Format, ...)
#endif
