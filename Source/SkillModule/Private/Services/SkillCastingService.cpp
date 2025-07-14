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

bool USkillCastingService::TryCastSkill(ACharacter* Caster, const FGuid& SlotId){
	// Client 기준으로 RPC -> 스킬 사용이 되어야함. 애초에 try ability는 predict 지원임 ㅇㅇ
	// if (!Caster || !Caster->HasAuthority()){
	// 	UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster is null. or not on server authority."));
	// 	return false;
	// }
	
	// 1. 컴포넌트 가져오기
	USkillComponent* SkillComp = Caster->GetPlayerState()->FindComponentByClass<USkillComponent>();
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Caster);
	if (!SkillComp || !ASC){
		UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster '%s' is missing required components (SkillComponent or AbilitySystemComponent)."), *Caster->GetName());
		return false;
	}
	
	USkillSlot* Slot = SkillComp->GetSkillSlotByGuid(SlotId);
	if (!Slot || !Slot->SkillData || !Slot->AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillCastingService: Invalid Slot or SkillData for SlotId, AbilityClass %s"), *SlotId.ToString());
		return false;
	}
	const FGameplayAbilitySpecHandle& AbilitySpec = ASC->FindAbilitySpecFromClass(Slot->AbilityClass)->Handle;

	// 8. Gameplay Ability 실행
	if (AbilitySpec.IsValid())
	{
		bool bIsActivated = ASC->TryActivateAbility(AbilitySpec);
		if (bIsActivated) {
			/*<-------------------- 스킬 사용 판정 이후 ------------------->*/
			// 도메인 이벤트 발행 -> GA 내부에서 결과에 따른 콜백을 사용합니다.
			// 상태 변경 (MarkUsed) -> GA에서 상태를 처리하므로 로직을 삭제.
			// 5. 영속화 (Repository)
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
							//TODO 현재 스킬상태를 DB에 저장하는것이 필요한지 검증.
							// if (!StateRepo->SaveSkillState(PlayerId, SkillComp, TODO))
							// {
							UE_LOG(LogTemp, Error, TEXT("SkillCastingService: FAILED to save skill state! Rolling back domain state."));
							// Slot->SetLastUsedTime(FDateTime::MinValue()); 
							return false; // 트랜잭션 실패
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