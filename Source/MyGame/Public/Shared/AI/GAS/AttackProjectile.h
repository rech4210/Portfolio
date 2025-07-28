#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shared/Player/GGwaCharacter.h"
#include "AttackProjectile.generated.h"

class AGameplayCueNotify_Actor;
class UGameplayEffect;
class UStaticMeshComponent;
class UParticleSystem;
class UProjectileMovementComponent;

UCLASS()
class MYGAME_API AAttackProjectile : public AActor {
	GENERATED_BODY()

public:
	AAttackProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, Category = "GE")
	TSubclassOf<UGameplayEffect> Projectile_GE;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float ArcHeight = 300.f;

	// ��ǥ �������� �����ϴ� �� �ɸ� �ð�(��)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float TravelTime = 1.5f;

	// Replicated start/end positions
	UPROPERTY(VisibleAnywhere, Replicated)
	FVector StartLocation;

	UPROPERTY(VisibleAnywhere, Replicated)
	FVector EndLocation;

	// �ʱ�ȭ
	void InitProjectile(const FVector& Start, AActor* Target);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							bool bFromSweep, const FHitResult& SweepResult);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	//������Ʈ Ǯ �����
	// ���� ����
	UPROPERTY(VisibleAnywhere)
	AGGwaCharacter* TargetCharacter;
	float CurrentTime;    // ��� �ð�
};
