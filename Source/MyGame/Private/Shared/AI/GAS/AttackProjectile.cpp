#include "Shared/AI/GAS/AttackProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayEffect.h"
#include "Animation/AnimTrace.h"
#include "Concepts/Iterable.h"
#include "Net/UnrealNetwork.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"

AAttackProjectile::AAttackProjectile() {
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetCollisionProfileName(TEXT("Projectile"));
    MeshComponent->SetNotifyRigidBodyCollision(true);
    MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AAttackProjectile::OnMeshBeginOverlap);
    RootComponent = MeshComponent;

    CurrentTime = 0.f;
}

void AAttackProjectile::BeginPlay() {
    Super::BeginPlay();
    SetReplicateMovement(true);
    SetActorTickEnabled(true);
}

void AAttackProjectile::InitProjectile(const FVector& Start, AActor* Target) {
    if (!HasAuthority()) return;

    if (auto Character = Cast<AGGwaCharacter>(Target)) {
        StartLocation = Start;
        TargetCharacter = Character;
        EndLocation = Target->GetActorLocation();
        CurrentTime = 0.f;
        SetActorLocation(StartLocation);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("AAttackProjectile::InitProjectile: Target is not a valid GGwaCharacter"));
    }
}

void AAttackProjectile::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    CurrentTime += DeltaSeconds;
    float t = FMath::Clamp(CurrentTime / TravelTime, 0.f, 1.f);

    if (!TargetCharacter) {
        SetActorLocation(EndLocation);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        // Destroy();
        return;
    }
    
    EndLocation = TargetCharacter->GetActorLocation();
    FVector HorizontalPos = FMath::Lerp(StartLocation, EndLocation, t);
    // ������ ������: zOffset = 4h * t * (1 - t)
    float zOffset = 4.f * ArcHeight * t * (1.f - t);
    FVector NewPos = HorizontalPos + FVector(0.f, 0.f, zOffset);

    SetActorLocation(NewPos);

    if (t >= 1.f) {
        Destroy();
    }
}

//��ô ��ġ�� VFX ����.
void AAttackProjectile::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                           bool bFromSweep, const FHitResult& SweepResult) {
    // check here for logic clarify
    if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
        return;

    if (HasAuthority() && Projectile_GE) {
        if (auto Character = Cast<AGGwaCharacter>(OtherActor)) {
            if (auto ASC = Character->GetAbilitySystemComponent()) {
                FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
                FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Projectile_GE, 1.f, Context);
                //GC ������ ���� �ӽ� ó��. ���� Duration ������� ó��
                if (SpecHandle.IsValid()) {
                    //Character�� after GE �����. ���⼭ ���׼� ����
                    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                }
            }
        }
    }
    Destroy();
}

void AAttackProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAttackProjectile, StartLocation);
    DOREPLIFETIME(AAttackProjectile, EndLocation);
}