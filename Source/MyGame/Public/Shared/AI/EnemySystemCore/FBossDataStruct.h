#pragma once

#include "CoreMinimal.h"
#include "FBossDataStruct.generated.h"

USTRUCT(BlueprintType)
struct MYGAME_API FBossDataStruct {
	GENERATED_BODY()

	UPROPERTY()
	int32 Phase;
	UPROPERTY()
	int32 Health;
	UPROPERTY()
	int32 Damage;
	
	FBossDataStruct& operator=(const FBossDataStruct& Other) {
		Phase = Other.Phase;
		Health = Other.Health;
		Damage = Other.Damage;
		return *this;
	}

	bool operator==(const FBossDataStruct& Other) const {
		return Phase == Other.Phase &&
			   Health == Other.Health &&
			   Damage == Other.Damage;
	}
};

