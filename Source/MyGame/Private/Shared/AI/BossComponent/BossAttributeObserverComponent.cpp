// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/AI/BossComponent/BossAttributeObserverComponent.h"
#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "MyGame/Public/Shared/AI/EnemyAttributeSet.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/EObservedAttribute.h"


UBossAttributeObserverComponent::UBossAttributeObserverComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UBossAttributeObserverComponent::BeginPlay() {
	Super::BeginPlay();
	AttributeHelper.AddHandler(EObservedAttribute::Health);
	AttributeHelper.AddHandler(EObservedAttribute::MaxHealth);
	AttributeHelper.AddHandler(EObservedAttribute::Damage);
}


//1. Shared Module 에 공용 인터페이스를 통해 연결한다.
//2. 현재 다양한 데이터 전송을 위해 struct 타입으로 전송하나, 전송할 데이터가 많아질경우 문제가 생길 수 있다.
//3. 현재 여러 데이터를 검증하고 전송하는 과정에서 과연... 해당 데이터를 처리하는 컨트롤러의 로직을 어떻게 수정할것인가.

void UBossAttributeObserverComponent::OnAttributeChanged(EObservedAttribute Attribute,
	const FOnAttributeChangeData& Data) const{
	ABossCharacter* BC = Cast<ABossCharacter>(GetOwner());
	const FBossDataStruct Old = BC->CachedBossData;
	AttributeHelper.HandleAttributeChange(Attribute, BC->CachedBossData, Data.NewValue);
	if (BC->HasAuthority()) {
		//주의해야할 점. Attribute의 값이 수정되지 않아도 callback 이 호출되는 경우가 존재했음. Old Value 기준으로 처리할것.
		if (Old == BC->CachedBossData) {
			return;
		}
		BC->UpdateDataFromBoss(BC->CachedBossData);
	}
}

	//GAS Attribute가 변경될 때 호출되는 Delegate에 함수를 바인딩.
void UBossAttributeObserverComponent::BindBossDataDelegate(){
	if (auto ASC = Cast<ABossCharacter>(GetOwner())->GetAbilitySystemComponent()) {
		ASC->GetGameplayAttributeValueChangeDelegate(UEnemyAttributeSet::GetHealthAttribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				OnAttributeChanged(EObservedAttribute::Health, Data);
			});

		ASC->GetGameplayAttributeValueChangeDelegate(UEnemyAttributeSet::GetMaxHealthAttribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				OnAttributeChanged(EObservedAttribute::MaxHealth, Data);
			});

		ASC->GetGameplayAttributeValueChangeDelegate(UEnemyAttributeSet::GetDamageAttribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				OnAttributeChanged(EObservedAttribute::Damage, Data);
			});
	}
}
