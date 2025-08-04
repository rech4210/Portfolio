


#include "Shared/AI/BossComponent/BossSkillComponent.h"


UBossSkillComponent::UBossSkillComponent() {
	PrimaryComponentTick.bCanEverTick = true;

}

void UBossSkillComponent::BeginPlay() {
	Super::BeginPlay();
}


void UBossSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

