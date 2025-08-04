#include "Shared/AI/BossComponent/BossStateComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

static constexpr int MIN_PHASE = 1;
static constexpr int MAX_PHASE = 3;

UBossStateComponent::UBossStateComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	CurrentBossState = EBossState::Idle;
}


void UBossStateComponent::BeginPlay() {
	Super::BeginPlay();
	UBlackboardComponent* BB =  Cast<AAIController>(GetOwner())->GetBlackboardComponent();
	if (BB){
		BindOnStateChanged(BB, EBossState::Idle);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("is not a AI Controller"));
	}
}

void UBossStateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBossStateComponent::UpdateBossState(UBlackboardComponent* BB, EBossState BossState) {
	if (CurrentBossState != BossState) {
		CurrentBossState = BossState;
	}
	OnStateChanged.Broadcast(BossState);
}


void UBossStateComponent::BindOnStateChanged(UBlackboardComponent* BB, EBossState NewState) {
}


void UBossStateComponent::AdvanceBossPhase(UBlackboardComponent* BB){
	UE_LOG(LogTemp, Warning, TEXT("Phase Change Request"));
	if (GetOwner()->HasAuthority()) {
		PhaseIndex = FMath::Clamp(PhaseIndex+1, MIN_PHASE, MAX_PHASE);;
		BB->SetValueAsInt(FName("PhaseIndex"), PhaseIndex);
	}
}







