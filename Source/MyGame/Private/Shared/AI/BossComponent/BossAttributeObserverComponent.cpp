// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/AI/BossComponent/BossAttributeObserverComponent.h"
#include "Shared/AI/BossCharacter.h"
#include "Shared/AI/EnemyAttributeSet.h"


// Sets default values for this component's properties
UBossAttributeObserverComponent::UBossAttributeObserverComponent() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBossAttributeObserverComponent::BeginPlay() {
	Super::BeginPlay();


}


//1. Shared Module 에 공용 인터페이스를 통해 연결한다.
//2. 현재 다양한 데이터 전송을 위해 struct 타입으로 전송하나, 전송할 데이터가 많아질경우 문제가 생길 수 있다.
//3. 현재 여러 데이터를 검증하고 전송하는 과정에서 과연... 해당 데이터를 처리하는 컨트롤러의 로직을 어떻게 수정할것인가.
void UBossAttributeObserverComponent::OnHealthChanged(const FOnAttributeChangeData& Data) const{
	ABossCharacter* BC = Cast<ABossCharacter>(GetOwner());
	FBossDataStruct BossData = {};
	BossData.Health = Data.NewValue;
	if (BC->HasAuthority()) {
		BC->UpdateDataFromBoss(BossData);
	}
}

void UBossAttributeObserverComponent::BindBossDataDelegate() {
	//GAS Attribute가 변경될 때 호출되는 Delegate에 함수를 바인딩.
	if (auto ASC = Cast<ABossCharacter>(GetOwner())->GetAbilitySystemComponent()) {
		ASC->GetGameplayAttributeValueChangeDelegate(UEnemyAttributeSet::GetHealthAttribute()).
		AddUObject(this,&ThisClass::OnHealthChanged);
	}
}
