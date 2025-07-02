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

bool USkillCastingService::TryCastSkill(AActor* Caster, const FGuid& SlotId){
	if (!Caster || !Caster->HasAuthority()){
		UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster is null. or not on server authority."));
		return false;
	}
	
	// 1. 컴포넌트 가져오기
	USkillComponent* SkillComp = Caster->FindComponentByClass<USkillComponent>();
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Caster);
	if (!SkillComp || !ASC){
		UE_LOG(LogTemp, Error, TEXT("SkillCastingService: Caster '%s' is missing required components (SkillComponent or AbilitySystemComponent)."), *Caster->GetName());
		return false;
	}
	
	USkillSlot* Slot = SkillComp->GetSkillSlot(SlotId);
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
			TScriptInterface<ISkillStateRepositoryInterface> StateRepo = GetSkillStateRepository(Caster);
			if (StateRepo)
			{
				APawn* Pawn = Cast<APawn>(Caster);
				int32 PlayerId = Pawn && Pawn->GetPlayerState() ? Pawn->GetPlayerState()->GetPlayerId() : -1;

				if (PlayerId != -1)
				{
					if (!StateRepo->SaveSkillState(PlayerId, SkillComp))
					{
						UE_LOG(LogTemp, Error, TEXT("SkillCastingService: FAILED to save skill state! Rolling back domain state."));
						// Slot->SetLastUsedTime(FDateTime::MinValue()); 
						return false; // 트랜잭션 실패
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

// 캐릭터에서 호출될거니까, 
TScriptInterface<ISkillStateRepositoryInterface> USkillCastingService::GetSkillStateRepository(const UObject* WorldContextObject) const
{
	// 실제 구현에서는 게임 인스턴스 서브시스템이나 다른 서비스 로케이터를 통해
	// ISkillStateRepository를 구현하는 구체 클래스의 인스턴스를 찾아야 합니다.
	if (GEngine)
	{
		// 예시: UGameInstance가 리포지토리 인터페이스를 구현한 경우
		// if (auto* GI = GEngine->GetGamePlayer(WorldContextObject, 0)->GetGameInstance())
		// {
		//	 if (GI->Implements<USkillStateRepository>())
		//	 {
		//		 return GI;
		//	 }
		// }
	}
	return nullptr;
}
//
// void USkillCastingService::PublishSkillUsedEvent(const FSkillUsedEvent& Event)
// {
// 	// 실제 구현에서는 등록된 모든 리스너에게 이벤트를 전달하는 이벤트 버스 시스템을 사용해야 합니다.
// 	// 이 부분은 GA의 태그 설정과 연동하도록 합시다.
// 	UE_LOG(LogTemp, Log, TEXT("SkillCastingService: Publishing FSkillUsedEvent for SlotId %s."), *Event.SlotId.ToString());
// 	// Example: FGameEventBus::Get().Publish(Event);
// } 