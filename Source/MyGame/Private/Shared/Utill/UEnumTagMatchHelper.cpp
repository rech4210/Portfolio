#include "Shared/Utill/UEnumTagMatchHelper.h"

#include "Shared/Data/EGasDataType.h"
#include "Shared/Player/EPlayerState.h"
#include "Shared/GAS/Cue/ECueType.h"
#include "Shared/GAS/EGasEventType.h"
#include "Shared/GAS/Skill/ESkillType.h"

template<>
MYGAME_API TMap<EObservedAttribute, FGameplayTag> UEnumTagMatchHelper::StateMap<EObservedAttribute>;
template<>
MYGAME_API TMap<EPlayerState, FGameplayTag> UEnumTagMatchHelper::StateMap<EPlayerState>;
template<>
MYGAME_API TMap<ECueType, FGameplayTag> UEnumTagMatchHelper::StateMap<ECueType>;
template<>
MYGAME_API TMap<EGasEventType, FGameplayTag> UEnumTagMatchHelper::StateMap<EGasEventType>;
template<>
MYGAME_API TMap<ESkillType, FGameplayTag> UEnumTagMatchHelper::StateMap<ESkillType>;
template<>
MYGAME_API TMap<EGasDataType, FGameplayTag> UEnumTagMatchHelper::StateMap<EGasDataType>;

void UEnumTagMatchHelper::InitializeHelper() {
	// Player States
	Register<EPlayerState>(EPlayerState::Idle, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Idle")));
	Register<EPlayerState>(EPlayerState::Combat, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Combat")));
	Register<EPlayerState>(EPlayerState::Stunned, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Stunned")));
	Register<EPlayerState>(EPlayerState::Knockback, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Knockback")));
	Register<EPlayerState>(EPlayerState::Dead, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Dead")));
	Register<EPlayerState>(EPlayerState::Hit, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Hit")));

	// Cue Types
	Register<ECueType>(ECueType::None, FGameplayTag::RequestGameplayTag(TEXT("Cue.None")));
	Register<ECueType>(ECueType::DirectionPreview, FGameplayTag::RequestGameplayTag(TEXT("Cue.DirectionPreview")));
	Register<ECueType>(ECueType::AreaAttackWarning, FGameplayTag::RequestGameplayTag(TEXT("Cue.AreaAttackWarning")));

	// GAS Event Types
	Register<EGasEventType>(EGasEventType::None, FGameplayTag::RequestGameplayTag(TEXT("GasEvent.None")));
	Register<EGasEventType>(EGasEventType::AbilityFinished, FGameplayTag::RequestGameplayTag(TEXT("GasEvent.AbilityFinished")));

	// Skill Types
	Register<ESkillType>(ESkillType::None, FGameplayTag::RequestGameplayTag(TEXT("Skill")));
	Register<ESkillType>(ESkillType::Attack, FGameplayTag::RequestGameplayTag(TEXT("Skill.Attack")));
	Register<ESkillType>(ESkillType::Skill1, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill1")));
	Register<ESkillType>(ESkillType::Skill2, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill2")));
	Register<ESkillType>(ESkillType::Skill3, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill3")));
	Register<ESkillType>(ESkillType::Skill4, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill4")));
	Register<ESkillType>(ESkillType::Skill5, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill5")));
	Register<ESkillType>(ESkillType::Skill6, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill6")));
	Register<ESkillType>(ESkillType::Skill7, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill7")));
	Register<ESkillType>(ESkillType::Skill8, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill8")));
	Register<ESkillType>(ESkillType::Skill9, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill9")));

	// GAS Data Types
	Register<EGasDataType>(EGasDataType::None, FGameplayTag::RequestGameplayTag(TEXT("Data.None")));
	Register<EGasDataType>(EGasDataType::Cooldown, FGameplayTag::RequestGameplayTag(TEXT("Data.Cooldown")));
	Register<EGasDataType>(EGasDataType::SkillID, FGameplayTag::RequestGameplayTag(TEXT("Data.SkillID")));
}
