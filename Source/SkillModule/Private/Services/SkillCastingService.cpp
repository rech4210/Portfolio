// @Needmodifi
#include "Services/SkillCastingService.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkillComponent.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "Events/SkillEvents.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "SkillSubsystem.h"
#include "GameFramework/Character.h"

bool USkillCastingService::TryCastSkill(ACharacter* Caster, int32 SlotIndex){
	// Client ê¸°ì??¼ë¡œ RPC -> ?¤í‚¬ ?¬ìš©???˜ì–´?¼í•¨. ? ì´ˆ??try ability??predict ì§€?ìž„ ?‡ã…‡
	// if (!Caster || !Caster->HasAuthority()){
	// 	UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster is null. or not on server authority."));
	// 	return false;
	// }
	
	// 1. ì»´í¬?ŒíŠ¸ ê°€?¸ì˜¤ê¸?
	USkillComponent* SkillComp = Caster->GetPlayerState()->FindComponentByClass<USkillComponent>();
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Caster);
	if (!SkillComp || !ASC){
		UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster '%s' is missing required components (SkillComponent or AbilitySystemComponent)."), *Caster->GetName());
		return false;
	}
	
	USkillSlot* Slot = SkillComp->GetSkillSlotByIndex(SlotIndex);
	if (!Slot || !Slot->SkillData || !Slot->SkillData->AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillCastingService: Invalid Slot or SkillData for SlotIndex %d"), SlotIndex);
		return false;
	}
	const FGameplayAbilitySpecHandle& AbilitySpec = ASC->FindAbilitySpecFromClass(Slot->SkillData->AbilityClass)->Handle;

	// 8. Gameplay Ability ?¤í–‰
	if (AbilitySpec.IsValid())
	{
		bool bIsActivated = ASC->TryActivateAbility(AbilitySpec);
		if (bIsActivated) {
			/*<-------------------- ?¤í‚¬ ?¬ìš© ?ì • ?´í›„ ------------------->*/
			// ?„ë©”???´ë²¤??ë°œí–‰ -> GA ?´ë??ì„œ ê²°ê³¼???°ë¥¸ ì½œë°±???¬ìš©?©ë‹ˆ??
			// ?íƒœ ë³€ê²?(MarkUsed) -> GA?ì„œ ?íƒœë¥?ì²˜ë¦¬?˜ë?ë¡?ë¡œì§???? œ.
			// 5. ?ì†??(Repository)
			if (Caster->GetGameInstance())
			{
				USkillSubsystem* SkillSubsystem = Caster->GetGameInstance()->GetSubsystem<USkillSubsystem>();
				if (SkillSubsystem)
				{
					TScriptInterface<ISkillRepositoryInterface> StateRepo = SkillSubsystem->GetSkillRepository();
					if (StateRepo)
					{
						APawn* Pawn = Cast<APawn>(Caster);
						int32 PlayerId = Pawn && Pawn->GetPlayerState() ? Pawn->GetPlayerState()->GetPlayerId() : -1;

						if (PlayerId != -1)
						{
							//TODO ?„ìž¬ ?¤í‚¬?íƒœë¥?DB???€?¥í•˜?”ê²ƒ???„ìš”?œì? ê²€ì¦?
							// if (!StateRepo->SaveSkillState(PlayerId, SkillComp, TODO))
							// {
							UE_LOG(LogTemp, Error, TEXT("SkillCastingService: FAILED to save skill state! Rolling back domain state."));
							// Slot->SetLastUsedTime(FDateTime::MinValue()); 
							return false; // ?¸ëžœ??…˜ ?¤íŒ¨
							// }
						}
					}
				}
			}
		}
		else {
			return false;
		}
	}
	
	return true;
} 