#include "Shared/Data/CombinedAbilityDataAsset.h"

void UCombinedAbilityDataAsset::SetData(USkillDataAsset* SkillData, UBuffDataAsset* BuffData, UItemDataAsset* ItemData) {
	SkillDataAsset = SkillData;
	BuffDataAsset = BuffData;
	ItemDataAsset = ItemData;
}
