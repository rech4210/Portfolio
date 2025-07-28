
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BattleModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SERVERMODULE_API UBattleModeBase : public UObject, public FTickableGameObject{
	GENERATED_BODY()
public:
	virtual void InitMode(){}
	virtual void StartBattle(){}
	virtual void EndBattle(){}
	virtual void ResetBattle(){}
	virtual void Tick(float DeltaTime) override{}
	virtual TStatId GetStatId() const override {
		return TStatId();
	}
};
