// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Utill/USkillHelper.h"


ESkillInputSlot USkillHelper::GetKeyFromIndex(int32 SlotIndex) {
    static const TArray<ESkillInputSlot> IndexToKey ={
        ESkillInputSlot::None,
            ESkillInputSlot::Q,
            ESkillInputSlot::W,
            ESkillInputSlot::E,
            ESkillInputSlot::R,
            ESkillInputSlot::A,
            ESkillInputSlot::S,
            ESkillInputSlot::D,
            ESkillInputSlot::F
        };

    return IndexToKey.IsValidIndex(SlotIndex) ? IndexToKey[SlotIndex] : ESkillInputSlot::None;
}

int USkillHelper::GetValueFromKey(ESkillInputSlot Slot){
    return static_cast<int>(Slot);
}


FString USkillHelper::GetKeyString(ESkillInputSlot InputSlot) {
    const UEnum *EnumPtr = StaticEnum<ESkillInputSlot>();
    return EnumPtr ? EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(InputSlot)).ToString() : FString("-");
}


