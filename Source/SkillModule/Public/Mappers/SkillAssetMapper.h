
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Mappers/ISkillAssetMapper.h"
#include "SkillAssetMapper.generated.h"

// Forward declarations
class USkillDataAsset;

/**
 * DTO ??DataAsset 매핑 구현�?
 * DTO?� UE5 DataAsset 간의 변?�을 구현
 * 주의: USkillDataAsset?� VO?��?�??�본 구조�??��?
 */
UCLASS(BlueprintType)
class SKILLMODULE_API USkillAssetMapper : public UObject, public ISkillAssetMapperInterface
{
	GENERATED_BODY()

public:
	USkillAssetMapper();

	// ISkillAssetMapperInterface interface
	virtual USkillDataAsset* MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO) override;
	virtual FSkillMasterDatabaseDTO MapDataAssetToDto(const USkillDataAsset* DataAsset) override;
	virtual USkillDataAsset* LoadOrCreateDataAsset(int32 SkillId) override;
	virtual void CacheDataAsset(int32 SkillId, USkillDataAsset* DataAsset) override;
	virtual USkillDataAsset* GetCachedDataAsset(int32 SkillId) override;
	virtual TArray<USkillDataAsset*> MapDtosToDataAssets(const TArray<FSkillMasterDatabaseDTO>& DTOs) override;
	virtual TArray<FSkillMasterDatabaseDTO> MapDataAssetsToDtos(const TArray<USkillDataAsset*>& DataAssets) override;
	virtual bool ValidateDataAsset(const USkillDataAsset* DataAsset, FString& OutErrorMessage) override;
	virtual bool ValidateDto(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) override;
	virtual FString GenerateAssetPath(int32 SkillId) override;
	virtual USkillDataAsset* LoadDataAssetFromPath(const FString& AssetPath) override;

protected:
	// DataAsset 캐시
	UPROPERTY()
	TMap<int32, USkillDataAsset*> CachedDataAssets;

	// Asset 경로 관�?
	UPROPERTY(EditDefaultsOnly, Category = "Asset Management")
	FString SkillDataAssetBasePath = TEXT("/Game/Data/Skills/");

private:
	// Helper functions
	void SyncDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO, USkillDataAsset* DataAsset);
	FSkillMasterDatabaseDTO ExtractDtoFromDataAsset(const USkillDataAsset* DataAsset);
};
