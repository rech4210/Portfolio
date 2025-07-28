
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Shared/Mode/ModeType.h"
#include "BaseGameMode.generated.h"

class AGGwaCharacter;
class AGGwaPlayerController;
class AGGwaPlayerState;

/**
 * 
 */
UCLASS()
class MYGAME_API ABaseGameMode : public AGameMode {
	GENERATED_BODY()
public:
	ABaseGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION()
	static void HandleAbilityActivated(const AActor* InstigatorActor,
									   FName AbilityName,
									   const FVector& TargetLocation);
protected:
	virtual void InitializeServerManagers() {}

	virtual void BeginPlay() override;

	virtual void RequestFlowControllerInit(EModeType ModeType) {}
	
};
