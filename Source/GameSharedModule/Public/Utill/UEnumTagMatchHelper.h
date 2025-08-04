#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/EGasDataType.h"
#include "Enum/ECueType.h"
#include "Enum/EGasEventType.h"
#include "Enum/EPlayerState.h"
#include "Enum/ESkillType.h"

class GAMESHAREDMODULE_API UEnumTagMatchHelper
{
public:
	static void InitializeHelper();
	static void Shutdown();

	static TMap<EPlayerState, FGameplayTag> PlayerStateMap;
	static TMap<ECueType, FGameplayTag> CueMap;
	static TMap<EGasEventType, FGameplayTag> EventMap;
	static TMap<ESkillType, FGameplayTag> SkillMap;
	static TMap<EGasDataType, FGameplayTag> DataMap;
	
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