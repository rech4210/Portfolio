// @Needmodifi
#include "Shared/GAS/Cue/UGCN_Skill1_DirectionPreview.h"
#include "SkillModule/Public/Data/SkillDataAsset.h"
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

AUGCN_Skill1_DirectionPreview::AUGCN_Skill1_DirectionPreview(){
	PrimaryActorTick.bCanEverTick = false;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); 
	DecalSize = FVector(0,0,0);
	DecalComponent->DecalSize = DecalSize;
}

bool AUGCN_Skill1_DirectionPreview::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!MyTarget || !BaseDecalMaterial){
		UE_LOG(LogTemp, Warning, TEXT("[Preview] Invalid Target or Material"));
		return false;
	}
	
	FPrimaryAssetId AssetId;
	ULocalDataBaseLoader::CheckPrimaryAssetId(Parameters.RawMagnitude, AssetId);
	SkillDataAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetId);
	if (!SkillDataAsset) {
		return false;
	}
	float MontageTimeRate = 0.f;
	if (!SkillDataAsset->CastMontage) {
		UE_LOG(LogTemp, Warning, TEXT("[Preview] CastMontage is null in SkillDataAsset"));
		return false;
	}
	MontageTimeRate = SkillDataAsset->CastMontage->GetPlayLength();
	ElapsedTime = 0.f;
	Duration = FMath::Clamp(MontageTimeRate, 0.0f, 10.f);

	FVector Location = Parameters.Location;
	FRotator Rotation = Parameters.Normal.Rotation();

	SetActorLocation(Location);
	SetActorRotation(Rotation);
	// ?°μΉΌ ?Έν…
	if (!DecalComponent) {
		DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
		DecalComponent->SetupAttachment(RootComponent);
		DecalComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); // μ§€λ©??μ‹??
	}
	auto BoxShapeConfig = SkillDataAsset->SkillShapeConfig.BoxHalfExtent;
	DecalComponent->DecalSize =  FVector(BoxShapeConfig.Z, BoxShapeConfig.Y, BoxShapeConfig.X); 
	DecalComponent->SetDecalMaterial(BaseDecalMaterial);

	UE_LOG(LogTemp, Log, TEXT("[Preview] Decal Size: %s"), *DecalComponent->DecalSize.ToString());

	UMaterialInterface* Material = DecalComponent->GetMaterial(0);
	DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DecalComponent->SetMaterial(0, DynMaterial);
	if (DynMaterial){
		UE_LOG(LogTemp, Log, TEXT("[Preview] DynMaterial created"));
		DynMaterial->SetScalarParameterValue(FName("Progress"), 0.f);
	}
	else{
		UE_LOG(LogTemp, Warning, TEXT("[Preview] DynMaterial creation failed"));
	}

	return true;
}

bool AUGCN_Skill1_DirectionPreview::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!MyTarget || !BaseDecalMaterial){
		UE_LOG(LogTemp, Warning, TEXT("[Preview] Invalid Target or Material"));
		return false;
	}

	FPrimaryAssetId AssetId;
	ULocalDataBaseLoader::CheckPrimaryAssetId(Parameters.RawMagnitude, AssetId);
	SkillDataAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetId);

	if (!SkillDataAsset) {
		return false;
	}
	float MontageTimeRate = 0.f;
	if (!SkillDataAsset->CastMontage) {
		UE_LOG(LogTemp, Warning, TEXT("[Preview] CastMontage is null in SkillDataAsset"));
		return false;
	}
	
	MontageTimeRate = SkillDataAsset->CastMontage->GetPlayLength();
	Duration = FMath::Clamp(MontageTimeRate, 0.1f, 10.f);
	ElapsedTime = 0.f;

	FVector Location = Parameters.Location;
	FRotator Rotation = Parameters.Normal.Rotation();
	SetActorLocation(Location);
	SetActorRotation(Rotation);

	DecalComponent->DecalSize = SkillDataAsset->SkillShapeConfig.BoxHalfExtent;
	DecalComponent->SetDecalMaterial(BaseDecalMaterial);

	auto* Material = DecalComponent->GetMaterial(0);
	DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DecalComponent->SetMaterial(0, DynMaterial);
	if (DynMaterial){
		DynMaterial->SetScalarParameterValue(FName("Progress"), 0.f);
	}

	GetWorld()->GetTimerManager().SetTimer(ProgressUpdateTimerHandle, this, &AUGCN_Skill1_DirectionPreview::UpdateProgress, 0.033f, true);

	return true;
}

void AUGCN_Skill1_DirectionPreview::UpdateProgress(){
	ElapsedTime += 0.066f;

	float Progress = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);
	if (DynMaterial){
		DynMaterial->SetScalarParameterValue(FName("Progress"), Progress);
	}

	UE_LOG(LogTemp, Log, TEXT("[Preview] Timer Progress = %.2f / %.2f"), ElapsedTime, Duration);

	if (ElapsedTime >= Duration){
		GetWorld()->GetTimerManager().ClearTimer(ProgressUpdateTimerHandle);
		Destroy();
	}
}

bool AUGCN_Skill1_DirectionPreview::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters){
	return true;
}

