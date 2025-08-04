
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "EnemySystemCore/FBossDataStruct.h"
#include "EnemySystemCore/FEnemyWidgetData.h"
#include "GameFramework/Character.h"
#include "BossCharacter.generated.h"

class UGameplayAbility;
class UBossAttributeObserverComponent;
class UBossSkillComponent;
class UEnemyAbilitySystemComponent;
class UEnemyAttributeSet;
class UEnemyDataAsset;

UCLASS()
class MYGAME_API ABossCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABossCharacter();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
	TArray<AActor*> DetectTarget(float Radius) const;

	UPROPERTY(ReplicatedUsing = OnRep_BossData)
	FBossDataStruct CachedBossData;

	UFUNCTION()
	void OnRep_BossData();
	void InitASC();
	void UpdateDataFromBoss(FBossDataStruct & Data);
	const FEnemyWidgetData& GetWidgetData();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UEnemyDataAsset> BossDataAsset;
private:
	TObjectPtr<UEnemyAbilitySystemComponent> E_ASC;
	TObjectPtr<UBossSkillComponent> SkillComponent;
	TObjectPtr<UEnemyAttributeSet> E_AttributeSet;
	
	TObjectPtr<UBossAttributeObserverComponent> AttributeObserverComponent;
	UPROPERTY(VisibleAnywhere)
	FEnemyWidgetData WidgetData;
};
