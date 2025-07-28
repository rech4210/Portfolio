

#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "MyGame/Public/Shared/AI/EnemyAbilitySystemComponent.h"
#include "MyGame/Public/Shared/AI/EnemyAttributeSet.h"
#include "MyGame/Public/Shared/AI/BossComponent/BossAttributeObserverComponent.h"
#include "MyGame/Public/Shared/AI/BossComponent/BossSkillComponent.h"
#include "MyGame/Public/Shared/AI/Interface/EnemyDataReceiver.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/EnemyDataAsset.h"

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

//?��?�?발동 ?�함.
void ABossCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
}


void ABossCharacter::BeginPlay(){
	Super::BeginPlay();
	WidgetData = BossDataAsset->WidgetData;
	const UEnemyAttributeSet* Attribute = E_ASC->GetSet<UEnemyAttributeSet>();
	CachedBossData = FBossDataStruct{1, Attribute->GetHealth(),Attribute->GetMaxHealth(),Attribute->GetDamage()};
}


/** 
 * 1. GAS Attribute 변�?발생 
 * 2. ObserverComponent?�서 ?�리게이?�로 감�? 
 * 3. OnAttributeChanged ??UpdateDataFromBoss ?�출 
 * 4. CachedBossData 갱신 
 * 5. ?�버 ???�라?�언??RPC: Client_ReceiveBossData 
 * 6. ?�라?�언?�에 ?�이???�기??
 */

void ABossCharacter::UpdateDataFromBoss(FBossDataStruct& Data) {
	if (HasAuthority()) {
		if (IEnemyDataReceiver* Receiver = Cast<IEnemyDataReceiver>(GetController())) {
			Receiver->ReceiveEnemyData(Data);
		}
	}
}



void ABossCharacter::OnRep_BossData() {
	// Client Base Replication
	// ForceNetUpdate()
	if (AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(UGameplayStatics::GetPlayerController(this, 0))) {
		// PC->InitializeClientComponent();
		PC->ProcessBossData(CachedBossData);
	}
}

const FEnemyWidgetData& ABossCharacter::GetWidgetData() {
	return WidgetData;
}

//Possesd?� Character??Init ?�점????고려?�것.
void ABossCharacter::InitASC() {
	if (!E_ASC)
	{
		E_ASC = CreateDefaultSubobject<UEnemyAbilitySystemComponent>(TEXT("E_ASC"));
	}
	if (E_ASC)
	{
		//보스??owner�?controller�?지?�해?�하??
		E_ASC->InitAbilityActorInfo(this, this);
	}
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


void ABossCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABossCharacter, CachedBossData);
}
