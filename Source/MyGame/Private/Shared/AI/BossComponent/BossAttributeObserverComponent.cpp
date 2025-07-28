

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


//1. Shared Module �� ���� �������̽��� ���� �����Ѵ�.
//2. ���� �پ��� ������ ������ ���� struct Ÿ������ �����ϳ�, ������ �����Ͱ� ��������� ������ ���� �� �ִ�.
//3. ���� ���� �����͸� �����ϰ� �����ϴ� �������� ����... �ش� �����͸� ó���ϴ� ��Ʈ�ѷ��� ������ ��� �����Ұ��ΰ�.

void UBossAttributeObserverComponent::OnAttributeChanged(EObservedAttribute Attribute,
	const FOnAttributeChangeData& Data) const{
	ABossCharacter* BC = Cast<ABossCharacter>(GetOwner());
	const FBossDataStruct Old = BC->CachedBossData;
	AttributeHelper.HandleAttributeChange(Attribute, BC->CachedBossData, Data.NewValue);
	if (BC->HasAuthority()) {
		//�����ؾ��� ��. Attribute�� ���� �������� �ʾƵ� callback �� ȣ��Ǵ� ��찡 ��������. Old Value �������� ó���Ұ�.
		if (Old == BC->CachedBossData) {
			return;
		}
		BC->UpdateDataFromBoss(BC->CachedBossData);
	}
}

	//GAS Attribute�� ����� �� ȣ��Ǵ� Delegate�� �Լ��� ���ε�.
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
