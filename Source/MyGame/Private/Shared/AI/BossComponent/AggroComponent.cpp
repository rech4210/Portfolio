


#include "Shared/AI/BossComponent/AggroComponent.h"


UAggroComponent::UAggroComponent() {
	PrimaryComponentTick.bCanEverTick = true;
}


void UAggroComponent::BeginPlay() {
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAggroComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                    FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

