#include "Utill/UEnumTagMatchHelper.h"

TMap<EPlayerState, FGameplayTag> UEnumTagMatchHelper::PlayerStateMap;
TMap<ECueType, FGameplayTag> UEnumTagMatchHelper::CueMap;
TMap<EGasEventType, FGameplayTag> UEnumTagMatchHelper::EventMap;
TMap<ESkillType, FGameplayTag> UEnumTagMatchHelper::SkillMap;
TMap<EGasDataType, FGameplayTag> UEnumTagMatchHelper::DataMap;

void UEnumTagMatchHelper::InitializeHelper()
{
	if (PlayerStateMap.Num() > 0)
	{
		return;
	}

	PlayerStateMap.Add(EPlayerState::Idle, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Idle")));
	PlayerStateMap.Add(EPlayerState::Combat, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Combat")));
	PlayerStateMap.Add(EPlayerState::Stunned, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Stunned")));
	PlayerStateMap.Add(EPlayerState::Knockback, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Knockback")));
	PlayerStateMap.Add(EPlayerState::Dead, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Dead")));
	PlayerStateMap.Add(EPlayerState::Hit, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Hit")));
	PlayerStateMap.Add(EPlayerState::Guard, FGameplayTag::RequestGameplayTag(TEXT("State.Player.Guard")));

	CueMap.Add(ECueType::None, FGameplayTag::RequestGameplayTag(TEXT("Cue.None")));
	CueMap.Add(ECueType::DirectionPreview, FGameplayTag::RequestGameplayTag(TEXT("Cue.DirectionPreview")));
	CueMap.Add(ECueType::AreaAttackWarning, FGameplayTag::RequestGameplayTag(TEXT("Cue.AreaAttackWarning")));
	
	EventMap.Add(EGasEventType::None, FGameplayTag::RequestGameplayTag(TEXT("GasEvent.None")));
	EventMap.Add(EGasEventType::AbilityFinished, FGameplayTag::RequestGameplayTag(TEXT("GasEvent.AbilityFinished")));
	EventMap.Add(EGasEventType::Notify, FGameplayTag::RequestGameplayTag(TEXT("GasEvent.Notify")));
	
	SkillMap.Add(ESkillType::None, FGameplayTag::RequestGameplayTag(TEXT("Skill")));
	SkillMap.Add(ESkillType::Attack, FGameplayTag::RequestGameplayTag(TEXT("Skill.Attack")));
	SkillMap.Add(ESkillType::Skill1, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill1")));
	SkillMap.Add(ESkillType::Skill2, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill2")));
	SkillMap.Add(ESkillType::Skill3, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill3")));
	SkillMap.Add(ESkillType::Skill4, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill4")));
	SkillMap.Add(ESkillType::Skill5, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill5")));
	SkillMap.Add(ESkillType::Skill6, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill6")));
	SkillMap.Add(ESkillType::Skill7, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill7")));
	SkillMap.Add(ESkillType::Skill8, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill8")));
	SkillMap.Add(ESkillType::Skill9, FGameplayTag::RequestGameplayTag(TEXT("Skill.Skill9")));
	
	DataMap.Add(EGasDataType::None, FGameplayTag::RequestGameplayTag(TEXT("Data.None")));
	DataMap.Add(EGasDataType::Cooldown, FGameplayTag::RequestGameplayTag(TEXT("Data.Cooldown")));
	DataMap.Add(EGasDataType::CueDuration, FGameplayTag::RequestGameplayTag(TEXT("Data.CueDuration")));
	DataMap.Add(EGasDataType::SkillID, FGameplayTag::RequestGameplayTag(TEXT("Data.SkillID")));
	DataMap.Add(EGasDataType::TagDuration, FGameplayTag::RequestGameplayTag(TEXT("Data.TagDuration")));
}

void UEnumTagMatchHelper::Shutdown()
{
	PlayerStateMap.Empty();
	CueMap.Empty();
	EventMap.Empty();
	SkillMap.Empty();
	DataMap.Empty();
}
