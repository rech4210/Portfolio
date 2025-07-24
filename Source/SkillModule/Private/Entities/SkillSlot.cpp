#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "Net/UnrealNetwork.h"

void USkillSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USkillSlot, SlotIndex);
    DOREPLIFETIME(USkillSlot, SkillId);
    DOREPLIFETIME(USkillSlot, SkillData);
    DOREPLIFETIME(USkillSlot, SlotKey);
}

void USkillSlot::Initialize(int32 InSlotIndex, const FString& InSlotKey, USkillDataAsset* InSkillData)
{
	SlotIndex = InSlotIndex;
	SlotKey = InSlotKey;
	SkillData = InSkillData;
	SkillId = InSkillData ? InSkillData->SkillID : 0;
	LastUsedTime = FDateTime::MinValue();
}

void USkillSlot::SetSkillData(USkillDataAsset* InSkillData, int32 InSkillId)
{
	SkillData = InSkillData;
	SkillId = InSkillId;
	LastUsedTime = FDateTime::MinValue(); // 새 스킬 설정 시 쿨타임 초기화
}

void USkillSlot::ClearSkill()
{
	SkillData = nullptr;
	SkillId = 0;
	LastUsedTime = FDateTime::MinValue();
}

bool USkillSlot::IsOnCooldown(float BaseCooltime) const
{
	if (BaseCooltime <= 0.0f || LastUsedTime <= FDateTime::MinValue())
	{
		return false;
	}

	FDateTime Now = FDateTime::Now();
	double ElapsedSeconds = (Now - LastUsedTime).GetTotalSeconds();
	return ElapsedSeconds < BaseCooltime;
}

float USkillSlot::GetRemainingCooldown(float BaseCooltime) const
{
	if (BaseCooltime <= 0.0f || LastUsedTime <= FDateTime::MinValue())
	{
		return 0.0f;
	}

	FDateTime Now = FDateTime::Now();
	double ElapsedSeconds = (Now - LastUsedTime).GetTotalSeconds();
	return FMath::Max(0.0f, BaseCooltime - static_cast<float>(ElapsedSeconds));
} 