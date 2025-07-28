#pragma once

#include "CoreMinimal.h"
#include "Enums/EClientUIKey.h"

/**
 * 클라이언트 UI 컴포넌트 매핑 관리
 * 간단한 문자열 기반 클래스 로딩 시스템
 */
class GAMESHAREDMODULE_API FClientUIMapping
{
public:
	/**
	 * UI 키에 해당하는 클래스 경로 반환
	 */
	static FString GetClassPath(EClientUIKey UIKey);
	
	/**
	 * UI 키에 해당하는 UClass 로딩 (클라이언트 전용)
	 */
	static UClass* LoadUIClass(EClientUIKey UIKey);
	
	/**
	 * 클래스 경로가 유효한지 확인
	 */
	static bool IsValidClassPath(const FString& ClassPath);
	
	/**
	 * 모든 매핑 정보 반환 (디버그용)
	 */
	static TMap<EClientUIKey, FString> GetAllMappings();

private:
	/**
	 * 기본 매핑 테이블 초기화
	 */
	static void InitializeDefaultMappings();
	
	/**
	 * 매핑 테이블 (키 -> 클래스 경로)
	 */
	static TMap<EClientUIKey, FString> UIClassMappings;
	
	/**
	 * 초기화 완료 플래그
	 */
	static bool bIsInitialized;
};
