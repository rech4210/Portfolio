#include "Data/SkillTargetBase.h"
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"

void USkillTargetBase::DebugSkillShape(const UWorld* World, const FVector& StartLocation, const FVector& EndLocation,
									   const FSkillShapeConfig& Config) const {
#if WITH_EDITOR
	// ?? Box Trace �ð�ȭ
	DrawDebugBox(
		World,
		(StartLocation + EndLocation) * 0.5f, // �ڽ� �߽�
		Config.BoxHalfExtent,
		FQuat::Identity,
		FColor::Blue,
		false,
		2.0f,  // ���� �ð� (��)
		0,
		2.0f   // �� �β�
	);

	DrawDebugSphere(
		World,
		EndLocation,
		Config.SphereRadius,
		12,            // ���׸�Ʈ ��
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
	//üũ �Ǵܿ��� ���� �����ؾ��Ѵ�. ����� character ��� ��ӹ޴´ٸ�, ������ �÷��̾� ��� Ÿ�� ����� �ȴ�.
	if (Actor->Implements<UAbilitySystemInterface>()) {
		return true;
	}
	return false;
}
