
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Engine/DataAsset.h"
#include "ISkillAssetMapper.generated.h"

class USkillDataAsset;

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class SKILLMODULE_API USkillAssetMapperInterface : public UInterface
{
	GENERATED_BODY()
};

class SKILLMODULE_API ISkillAssetMapperInterface
{
	GENERATED_BODY()

public:
	virtual USkillDataAsset* MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO) = 0;
	
	virtual FSkillMasterDatabaseDTO MapDataAssetToDto(const USkillDataAsset* DataAsset) = 0;

	virtual USkillDataAsset* LoadOrCreateDataAsset(int32 SkillId) = 0;
	virtual void CacheDataAsset(int32 SkillId, USkillDataAsset* DataAsset) = 0;
	virtual USkillDataAsset* GetCachedDataAsset(int32 SkillId) = 0;

	virtual TArray<USkillDataAsset*> MapDtosToDataAssets(const TArray<FSkillMasterDatabaseDTO>& DTOs) = 0;
	virtual TArray<FSkillMasterDatabaseDTO> MapDataAssetsToDtos(const TArray<USkillDataAsset*>& DataAssets) = 0;

	virtual bool ValidateDataAsset(const USkillDataAsset* DataAsset, FString& OutErrorMessage) = 0;
	virtual bool ValidateDto(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) = 0;

	virtual FString GenerateAssetPath(int32 SkillId) = 0;
	virtual USkillDataAsset* LoadDataAssetFromPath(const FString& AssetPath) = 0;
};
