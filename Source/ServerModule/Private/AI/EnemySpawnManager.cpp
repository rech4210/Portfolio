// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemySpawnManager.h"

#include "Shared/AI/BossCharacter.h"


// Sets default values
AEnemySpawnManager::AEnemySpawnManager() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemySpawnManager::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) {
		auto boss = GetWorld()->SpawnActor<ABossCharacter>(BossClass,Location, FRotator::ZeroRotator);
		boss->SpawnDefaultController();
	}
}

// Called every frame
void AEnemySpawnManager::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

