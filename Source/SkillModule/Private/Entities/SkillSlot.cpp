#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "Net/UnrealNetwork.h"

void USkillSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USkillSlot, SlotId);
    DOREPLIFETIME(USkillSlot, SkillData);
    DOREPLIFETIME(USkillSlot, AbilityClass);
}



void USkillSlot::Initialize(USkillDataAsset* InSkillData, TSubclassOf<UGameplayAbility> InAbilityClass)
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