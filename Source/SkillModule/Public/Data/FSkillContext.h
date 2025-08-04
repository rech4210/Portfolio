#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Data/SkillDataAsset.h"
#include "FSkillContext.generated.h"

USTRUCT(BlueprintType)
struct FSkillContext
{
	GENERATED_BODY()

	FSkillContext() = default;

	FSkillContext(UAbilitySystemComponent* InSourceASC, AActor* InSourceActor, AActor* InTargetActor, const FVector& InTargetLocation, USkillDataAsset* InSkillData, int32 InComboIndex = 0, float InStartTime = 0.f);
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TObjectPtr<AActor> SourceActor;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	TArray<AActor*> DetectedActors;

	UPROPERTY()
	TObjectPtr<USkillDataAsset> SkillData;

	UPROPERTY()
	FVector HitLocation;

	UPROPERTY()
	int32 ComboIndex = 0;

	UPROPERTY()
	float StartTime = 0.f;
};
