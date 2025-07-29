#include "Services/SkillCastingService.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkillComponent.h"
#include "Data/SkillDataAsset.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Utill/LocalDataBaseLoader.h"

bool USkillCastingService::TryCastSkill(ACharacter* Caster, int32 SlotIndex){

	// 1. 컴포넌트 가져오기
	USkillComponent* SkillComp = Caster->GetPlayerState()->FindComponentByClass<USkillComponent>();
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Caster);
	if (!SkillComp || !ASC){
		UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster '%s' is missing required components (SkillComponent or AbilitySystemComponent)."), *Caster->GetName());
		return false;
	}
	const FSkillSlotReplicationData* SlotData = SkillComp->GetSkillSlotDataByIndex(SlotIndex);

	if (SlotData->IsEmpty()) {
		return false;
	}
	
	USkillDataAsset* SkillDataAsset = SlotData->SkillData;
	if (!SkillDataAsset || !SkillDataAsset->AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillCastingService: Invalid SkillDataAsset for SkillId %d"), SlotData->SkillId);
		FPrimaryAssetId AssetID;
		ULocalDataBaseLoader::CheckPrimaryAssetId(SlotData->SkillId, AssetID);
		SkillDataAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetID);
	}

	const FGameplayAbilitySpecHandle& AbilitySpec = ASC->FindAbilitySpecFromClass(SkillDataAsset->AbilityClass)->Handle;

	if (AbilitySpec.IsValid())
	{
		bool bIsActivated = ASC->TryActivateAbility(AbilitySpec, true);
		if (bIsActivated) {
			// Rollback Logic
		}
		else {
			return false;
		}
	}
	
	return true;
} 