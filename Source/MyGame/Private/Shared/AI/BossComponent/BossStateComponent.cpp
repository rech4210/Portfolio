


#include "Shared/AI/BossComponent/BossStateComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

static constexpr int MIN_PHASE = 1;
static constexpr int MAX_PHASE = 3;

// Sets default values for this component's properties
UBossStateComponent::UBossStateComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	CurrentBossState = EBossState::Idle;
}


// Called when the game starts
void UBossStateComponent::BeginPlay() {
	Super::BeginPlay();

}


// Called every frame
void UBossStateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBossStateComponent::UpdateBossState(UBlackboardComponent* BB, EBossState BossState) {
}

void UBossStateComponent::AdvanceBossPhase(UBlackboardComponent* BB){
	PhaseIndex = FMath::Clamp(PhaseIndex+1, MIN_PHASE, MAX_PHASE);;
	BB->SetValueAsInt(FName("PhaseIndex"), PhaseIndex);
}









