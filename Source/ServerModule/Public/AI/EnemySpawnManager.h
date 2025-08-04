
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnManager.generated.h"

class ABossCharacter;
UCLASS()
class SERVERMODULE_API AEnemySpawnManager : public AActor {
	GENERATED_BODY()

public:
	AEnemySpawnManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TSubclassOf<ABossCharacter> BossClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	FVector Location;
};
