#pragma once

#include "CoreMinimal.h"
#include "Shared/Mode/BaseGameMode.h"
#include "LoginGameMode.generated.h"

UCLASS()
class CLIENTMODULE_API ALoginGameMode : public ABaseGameMode {
	GENERATED_BODY()
public:
	ALoginGameMode();
	virtual void Tick(float DeltaSeconds) override;
};
