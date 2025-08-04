
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "Shared/AI/EnemySystemCore/EObservedAttribute.h"
#include "Shared/Utill/FObservedAttributeHelper.h"
#include "BossAttributeObserverComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYGAME_API UBossAttributeObserverComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UBossAttributeObserverComponent();

protected:
	virtual void BeginPlay() override;

	FObservedAttributeHelper<FBossDataStruct> AttributeHelper;
public:
	void OnAttributeChanged(EObservedAttribute Attribute, const FOnAttributeChangeData& Data) const;
	void BindBossDataDelegate();
};
