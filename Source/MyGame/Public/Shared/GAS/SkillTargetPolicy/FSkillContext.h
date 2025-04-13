#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Shared/Data/SkillDataAsset.h"
#include "FSkillContext.generated.h"

USTRUCT(BlueprintType)
struct FSkillContext
{
	GENERATED_BODY()

public:
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

	/** 콤보 인덱스 (콤보 기반 스킬용) */
	UPROPERTY()
	int32 ComboIndex = 0;

	/** 스킬이 시작된 시점 시간 (후처리 로직 등에 활용 가능) */
	UPROPERTY()
	float StartTime = 0.f;
};
