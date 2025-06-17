// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Player/Component/PlayerSkillComponent.h"


// Sets default values for this component's properties
UPlayerSkillComponent::UPlayerSkillComponent() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerSkillComponent::BeginPlay() {
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlayerSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

