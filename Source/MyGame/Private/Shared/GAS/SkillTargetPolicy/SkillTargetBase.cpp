
#include "Shared/GAS/SkillTargetPolicy/SkillTargetBase.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "Shared/AI/BossCharacter.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"

void USkillTargetBase::DebugSkillShape(const UWorld* World, const FVector& StartLocation, const FVector& EndLocation,
                                       const FSkillShapeConfig& Config) const {
#if WITH_EDITOR
	// 🔷 Box Trace 시각화
	DrawDebugBox(
		World,
		(StartLocation + EndLocation) * 0.5f, // 박스 중심
		Config.BoxHalfExtent,
		FQuat::Identity,
		FColor::Blue,
		false,
		2.0f,  // 지속 시간 (초)
		0,
		2.0f   // 선 두께
	);

	DrawDebugSphere(
		World,
		EndLocation,
		Config.SphereRadius,
		12,            // 세그먼트 수
		FColor::Green,
		false,
		2.0f,
		0,
		2.0f
	);

	DrawDebugLine(
		World,
		StartLocation,
		EndLocation,
		FColor::Red,
		false,
		2.0f,
		0,
		1.0f
	);
#endif
}

bool USkillTargetBase::HasASC(AActor* Actor) const {
	//체크 판단에서 적을 정의해야한다. 현재는 character 모두 상속받는다면, 보스와 플레이어 모두 타격 대상이 된다.
	if (AGGwaCharacter* Character =  Cast<AGGwaCharacter>(Actor)) {
		if (Character->Implements<UAbilitySystemInterface>()) {
			return true;
		}
		return false;
	}
	if (ABossCharacter* Boss = Cast<ABossCharacter>(Actor)) {
		if (UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent()) {
			return true;
		}
		return false;
	}
	return false;
}
