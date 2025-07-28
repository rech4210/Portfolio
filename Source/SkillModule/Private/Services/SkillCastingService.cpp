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
	// Client 기�??�로 RPC -> ?�킬 ?�용???�어?�함. ?�초??try ability??predict 지?�임 ?�ㅇ
	// if (!Caster || !Caster->HasAuthority()){
	// 	UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster is null. or not on server authority."));
	// 	return false;
	// }
	
	// 1. 컴포?�트 가?�오�?
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

	// 8. Gameplay Ability ?�행
	if (AbilitySpec.IsValid())
	{
		bool bIsActivated = ASC->TryActivateAbility(AbilitySpec);
		if (bIsActivated) {
			/*<-------------------- ?�킬 ?�용 ?�정 ?�후 ------------------->*/
			// ?�메???�벤??발행 -> GA ?��??�서 결과???�른 콜백???�용?�니??
			// ?�태 변�?(MarkUsed) -> GA?�서 ?�태�?처리?��?�?로직????��.
			// 5. ?�속??(Repository)
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
							//TODO ?�재 ?�킬?�태�?DB???�?�하?�것???�요?��? 검�?
							// if (!StateRepo->SaveSkillState(PlayerId, SkillComp, TODO))
							// {
							UE_LOG(LogTemp, Error, TEXT("SkillCastingService: FAILED to save skill state! Rolling back domain state."));
							// Slot->SetLastUsedTime(FDateTime::MinValue()); 
							return false; // ?�랜??�� ?�패
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