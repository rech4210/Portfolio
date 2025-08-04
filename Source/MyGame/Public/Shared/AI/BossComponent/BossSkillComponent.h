#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossSkillComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UBossSkillComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UBossSkillComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
