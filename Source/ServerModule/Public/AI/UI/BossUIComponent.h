
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossUIComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SERVERMODULE_API UBossUIComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UBossUIComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
