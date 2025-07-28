// @Needmodifi
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/EGasDataType.h"
#include "Enum/ECueType.h"
#include "Enum/EGasEventType.h"
#include "Enum/EPlayerState.h"
#include "Enum/ESkillType.h"

/**
 * UEnumTagMatchHelper
 *
 * Unreal Engine의 모듈 시스템에서는 템플릿 static 멤버(StateMap<T>)를 사용할 경우,
 * 각 모듈(예: MyGame, ServerModule)마다 별도의 인스턴스가 생성되어 링커 오류(LNK2001) 및 데이터 불일치가 발생할 수 있습니다.
 * 따라서 enum별로 명시적으로 static TMap을 선언하여 모든 모듈에서 동일한 메모리 공간을 참조하도록 설계하였습니다.
 *
 * - 기존 template static 방식: enum 추가 시 헤더에 정의만 추가하면 되지만, 모듈별로 분리되어 링커 오류 발생
 * - 명시적 static 멤버 방식: enum별로 static 멤버를 선언하여 모듈 전체에서 공유, 링커 오류 및 데이터 불일치 해결
 *
 * 
 */
class GAMESHAREDMODULE_API UEnumTagMatchHelper
{
public:
	static void InitializeHelper();
	static void Shutdown();

	/**
	 * enum 타입에 따라 해당하는 TMap에서 FGameplayTag를 반환합니다.
	 * (기존 명칭 유지: GetTagFromEnum)
	 */
	// 아래 static 맵들은 Unreal 모듈 시스템에서의 링커 오류 및 데이터 불일치 방지를 위해 명시적으로 선언합니다.
	static TMap<EPlayerState, FGameplayTag> PlayerStateMap; ///< PlayerState용 태그 맵
	static TMap<ECueType, FGameplayTag> CueMap;             ///< CueType용 태그 맵
	static TMap<EGasEventType, FGameplayTag> EventMap;      ///< GAS EventType용 태그 맵
	static TMap<ESkillType, FGameplayTag> SkillMap;         ///< SkillType용 태그 맵
	static TMap<EGasDataType, FGameplayTag> DataMap;        ///< GAS DataType용 태그 맵
	
	template <typename T>
	static FGameplayTag GetTagFromEnum(const T& InEnum);
};

template <typename T>
FGameplayTag UEnumTagMatchHelper::GetTagFromEnum(const T& InEnum)
{
	if constexpr (std::is_same_v<T, EPlayerState>)
	{
		if (const FGameplayTag* FoundTag = PlayerStateMap.Find(InEnum)) { return *FoundTag; }
	}
	else if constexpr (std::is_same_v<T, ECueType>)
	{
		if (const FGameplayTag* FoundTag = CueMap.Find(InEnum)) { return *FoundTag; }
	}
	else if constexpr (std::is_same_v<T, EGasEventType>)
	{
		if (const FGameplayTag* FoundTag = EventMap.Find(InEnum)) { return *FoundTag; }
	}
	else if constexpr (std::is_same_v<T, ESkillType>)
	{
		if (const FGameplayTag* FoundTag = SkillMap.Find(InEnum)) { return *FoundTag; }
	}
	else if constexpr (std::is_same_v<T, EGasDataType>)
	{
		if (const FGameplayTag* FoundTag = DataMap.Find(InEnum)) { return *FoundTag; }
	}

	return FGameplayTag::EmptyTag;
}