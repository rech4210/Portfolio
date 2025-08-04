#include "AI/UI/BossUIComponent.h"

UBossUIComponent::UBossUIComponent() {
	PrimaryComponentTick.bCanEverTick = true;
}


void UBossUIComponent::BeginPlay() {
	Super::BeginPlay();
}


void UBossUIComponent::TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


