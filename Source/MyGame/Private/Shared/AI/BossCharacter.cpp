// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/AI/BossCharacter.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"
#include "Shared/AI/EnemyAttributeSet.h"
#include "Shared/AI/BossComponent/BossAttributeObserverComponent.h"
#include "Shared/AI/BossComponent/BossSkillComponent.h"
#include "Shared/AI/Interface/EnemyDataReceiver.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"

static constexpr int MAX_FLOAT = 9999.f;

UAbilitySystemComponent* ABossCharacter::GetAbilitySystemComponent() const {
	return Cast<UAbilitySystemComponent>(E_ASC.Get());
}

ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	E_ASC = CreateDefaultSubobject<UEnemyAbilitySystemComponent>(TEXT("E_ASC"));
	E_ASC->SetIsReplicated(true);
	E_ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	E_AttributeSet = CreateDefaultSubobject<UEnemyAttributeSet>(TEXT("E_AttributeSet"));
	
	SkillComponent = CreateDefaultSubobject<UBossSkillComponent>(TEXT("SkillComponent"));
	AttributeObserverComponent = CreateDefaultSubobject<UBossAttributeObserverComponent>(TEXT("AttributeObserverComponent"));
}



void ABossCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitASC();
	
	for (auto GA : Abilities) {
		FGameplayAbilitySpec Spec(GA, 1);
		E_ASC->GiveAbility(Spec);
	}
	E_ASC->RefreshAbilityActorInfo();
	AttributeObserverComponent->BindBossDataDelegate();
}

//이부분 발동 안함.
void ABossCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
}

void ABossCharacter::UpdateDataFromBoss(FBossDataStruct& Data) {
	if (HasAuthority()) {
		// 데이터 미변경시에 강제 복제 실시
		// if (CachedBossData == Data) {
		// 	ForceNetUpdate();
		//	CachedBossData = Data;
		// }
		if (!(CachedBossData == Data)) {
			CachedBossData = Data;
			if (IEnemyDataReceiver* Receiver = Cast<IEnemyDataReceiver>(GetController())) {
				Receiver->ReceiveEnemyData(Data);
			}
		}
	}
}

void ABossCharacter::OnRep_BossData() {
	// Client Base Replication
	// ForceNetUpdate()
	if (AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(UGameplayStatics::GetPlayerController(this, 0))) {
		PC->Client_ReceiveBossData(CachedBossData);
	}
}

void ABossCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABossCharacter, CachedBossData);
}

//Possesd와 Character의 Init 시점을 잘 고려할것.
void ABossCharacter::InitASC() {
	if (!E_ASC)
	{
		E_ASC = CreateDefaultSubobject<UEnemyAbilitySystemComponent>(TEXT("E_ASC"));
	}
	if (E_ASC)
	{
		E_ASC->InitAbilityActorInfo(this, this);
	}
}

void ABossCharacter::BeginPlay(){
	Super::BeginPlay();
}

// TODO: Be Componentize.
TArray<AActor*> ABossCharacter::DetectTarget(float Radius) const{
	TArray<AActor*> DetectedActors;

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());
	
	UKismetSystemLibrary::CapsuleOverlapActors(
		GetWorld(),
		GetActorLocation(),
		Radius, 
		Radius,
		{UEngineTypes::ConvertToObjectType(ECC_Pawn)},
		AGGwaCharacter::StaticClass(),
		IgnoreActors,
		DetectedActors
	);

	TArray<AActor*> LoopSafeCopyArray;
	// collision
	if (DetectedActors.Num() > 0) {
		float TargetDistance = MAX_FLOAT;
		for (auto DetectedActor : DetectedActors /*-> Collision Target*/) {
			if (Cast<AGGwaCharacter>(DetectedActor)) {
				float Distance = DetectedActor->GetDistanceTo(this);
				if (Distance < TargetDistance) {
					TargetDistance = Distance;
					LoopSafeCopyArray.Add(DetectedActor);
				}
			}
		}
	}
	Algo::Sort(LoopSafeCopyArray,[this](AActor* A, AActor* B) {
		return A->GetDistanceTo(this) < B->GetDistanceTo(this);	
	});
	
	// bool bActivated = E_ASC->TryActivateAbilityByClass(Abilities[0], true);
	
	return LoopSafeCopyArray;
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABossCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

