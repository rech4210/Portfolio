#pragma once

#include "AbilityInputID.generated.h"


UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
    None UMETA(displayname = "None"),
    Confirm UMETA(displayname = "Confirm"),
    Cancel UMETA(displayname = "Cancel"),
    Move UMETA(displayname = "Move"),
    Skill1 UMETA(displayname = "Skill1"),
    Skill2 UMETA(displayname = "Skill2"),
    Skill3 UMETA(displayname = "Skill3"),
    Skill4 UMETA(displayname = "Skill4"),
    Skill5 UMETA(displayname = "Skill5"),
    Skill6 UMETA(displayname = "Skill6"),
    Skill7 UMETA(displayname = "Skill7"),
    Skill8 UMETA(displayname = "Skill8")
};
