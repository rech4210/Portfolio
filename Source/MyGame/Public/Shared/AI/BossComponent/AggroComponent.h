#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AggroComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UAggroComponent : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAggroComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
