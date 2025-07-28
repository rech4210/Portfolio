

#include "AI/UI/BossUIComponent.h"


UBossUIComponent::UBossUIComponent() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


void UBossUIComponent::BeginPlay() {
	Super::BeginPlay();
	// BossWidget = CreateWidget<UBossWidget>(GetOwner(), BossWidgetClass);
	// BossWidget->AddToViewport();
}


void UBossUIComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


