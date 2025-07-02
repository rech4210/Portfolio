#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"

void USkillSlot::Initialize(const USkillDataAsset* InSkillData, TSubclassOf<UGameplayAbility> InAbilityClass)
{
	SlotId = FGuid::NewGuid();
	SkillData = InSkillData;
	AbilityClass = InAbilityClass;
	LastUsedTime = FDateTime::MinValue();
}
//
// void USkillSlot::SetLastUsedTime(const FDateTime& InTime)
// {
// 	LastUsedTime = InTime;
// }
//
// bool USkillSlot::IsOnCooldown(const FDateTime& CurrentTime) const
// {
// 	if (!SkillData)
// 	{
// 		return false;
// 	}
//
// 	const FTimespan CooldownDuration = FTimespan::FromSeconds(SkillData->CoolTime);
// 	const FDateTime CooldownEndTime = LastUsedTime + CooldownDuration;
//
// 	return CurrentTime < CooldownEndTime;
// } 