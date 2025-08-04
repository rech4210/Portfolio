#include "AI/EnemySpawnManager.h"
#include "Shared/AI/BossCharacter.h"

AEnemySpawnManager::AEnemySpawnManager() {
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySpawnManager::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) {
		auto boss = GetWorld()->SpawnActor<ABossCharacter>(BossClass,Location, FRotator::ZeroRotator);
		boss->SpawnDefaultController();
	}
}

void AEnemySpawnManager::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

