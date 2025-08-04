#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AggroComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UAggroComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UAggroComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
