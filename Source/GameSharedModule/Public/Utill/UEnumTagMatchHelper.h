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
 * Unreal Engine�� ��� �ý��ۿ����� ���ø� static ���(StateMap<T>)�� ����� ���,
 * �� ���(��: MyGame, ServerModule)���� ������ �ν��Ͻ��� �����Ǿ� ��Ŀ ����(LNK2001) �� ������ ����ġ�� �߻��� �� �ֽ��ϴ�.
 * ���� enum���� ��������� static TMap�� �����Ͽ� ��� ��⿡�� ������ �޸� ������ �����ϵ��� �����Ͽ����ϴ�.
 *
 * - ���� template static ���: enum �߰� �� ����� ���Ǹ� �߰��ϸ� ������, ��⺰�� �и��Ǿ� ��Ŀ ���� �߻�
 * - ����� static ��� ���: enum���� static ����� �����Ͽ� ��� ��ü���� ����, ��Ŀ ���� �� ������ ����ġ �ذ�
 *
 * 
 */
class GAMESHAREDMODULE_API UEnumTagMatchHelper
{
public:
	static void InitializeHelper();
	static void Shutdown();

	/**
	 * enum Ÿ�Կ� ���� �ش��ϴ� TMap���� FGameplayTag�� ��ȯ�մϴ�.
	 * (���� ��Ī ����: GetTagFromEnum)
	 */
	// �Ʒ� static �ʵ��� Unreal ��� �ý��ۿ����� ��Ŀ ���� �� ������ ����ġ ������ ���� ��������� �����մϴ�.
	static TMap<EPlayerState, FGameplayTag> PlayerStateMap; ///< PlayerState�� �±� ��
	static TMap<ECueType, FGameplayTag> CueMap;             ///< CueType�� �±� ��
	static TMap<EGasEventType, FGameplayTag> EventMap;      ///< GAS EventType�� �±� ��
	static TMap<ESkillType, FGameplayTag> SkillMap;         ///< SkillType�� �±� ��
	static TMap<EGasDataType, FGameplayTag> DataMap;        ///< GAS DataType�� �±� ��
	
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