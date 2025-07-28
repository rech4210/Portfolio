// @Needmodifi
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shared/AI/EnemySystemCore/BossPhaseTypes.h"
#include "Shared/AI/EnemySystemCore/BossStateFlags.h"
#include "BossStateComponent.generated.h"

class UBlackboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, EBossState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UBossStateComponent : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBossStateComponent();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
	void UpdateBossState(UBlackboardComponent* BB, EBossState BossState);
	void AdvanceBossPhase(UBlackboardComponent* BB);
	
	UPROPERTY(BlueprintAssignable)
	FOnStateChanged OnStateChanged;

	void BindOnStateChanged(UBlackboardComponent* BB, EBossState NewState);
private:
	EBossState CurrentBossState;
	int PhaseIndex = 1;
};
