


#include "Shared/AI/BossComponent/BossPhaseComponent.h"


UBossPhaseComponent::UBossPhaseComponent() {
	PrimaryComponentTick.bCanEverTick = true;

}


void UBossPhaseComponent::BeginPlay() {
	Super::BeginPlay();

}


void UBossPhaseComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

