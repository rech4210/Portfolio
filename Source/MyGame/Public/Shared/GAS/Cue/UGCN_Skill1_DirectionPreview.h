#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Looping.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UGCN_Skill1_DirectionPreview.generated.h"

class USkillDataAsset;
class ULocalDataBaseLoader;
UCLASS()
class MYGAME_API AUGCN_Skill1_DirectionPreview : public AGameplayCueNotify_Looping
{
	GENERATED_BODY()

public:
	AUGCN_Skill1_DirectionPreview();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Cue|Decal")
	UMaterialInterface* BaseDecalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Cue|Decal")
	FVector DecalSize;

	UPROPERTY(EditDefaultsOnly, Category = "Cue|Timing")
	float Duration;

private:
	bool bIsPlaying;
	UPROPERTY()
	UDecalComponent* DecalComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* DynMaterial;

	TObjectPtr<USkillDataAsset> SkillDataAsset;
	
	FTimerHandle ProgressUpdateTimerHandle;

	float ElapsedTime = 0.f;
	void UpdateProgress();
};
