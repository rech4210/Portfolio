
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "EnemyAbilitySystemComponent.generated.h"

/**
 * 
 */		
UCLASS()
class MYGAME_API UEnemyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	UEnemyAbilitySystemComponent();

	virtual void BeginPlay() override;
	
};
