#pragma once

#include "GameplayTagContainer.h"

class MYGAME_API UEnumTagMatchHelper {
	public:
#if __cpp_concepts >= 201907L
	template<typename T>
	requires std::is_enum_v<T>
	static void Register(T State, FGameplayTag Tag) {
		StateMap<T>.Add(State, Tag);
	}
#else
	template<typename T>
	typename std::enable_if<std::is_enum<T>::value, void>::type
	static Register(T State, FGameplayTag Tag) -> auto {
		StateMap<T>.Add(State, Tag);
	}
#endif

	static void InitializeHelper();
	template<typename T>
	static FGameplayTag GetTagFromEnum(const T& State){
		if (const FGameplayTag* Tag = StateMap<T>.Find(State)) {
			return *Tag;
		}
		return FGameplayTag::EmptyTag;
	}
private:
	template<typename T>
	static inline TMap<T, FGameplayTag> StateMap;
};