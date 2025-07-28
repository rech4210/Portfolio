// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Engine/DataAsset.h"
#include "ISkillAssetMapper.generated.h"

// Forward declarations
class USkillDataAsset;

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class SKILLMODULE_API USkillAssetMapperInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * DTO ??DataAsset Îß§Ìïë ?∏ÌÑ∞?òÏù¥??
 * DTO?Ä UE5 DataAsset Í∞ÑÏùò Î≥Ä?òÏùÑ ?¥Îãπ
 * Ï£ºÏùò: USkillDataAsset?Ä VO(Value Object)?¥Î?Î°??êÎ≥∏ Íµ¨Ï°∞Î•??†Ï??¥Ïïº ??
 */
class SKILLMODULE_API ISkillAssetMapperInterface
{
	GENERATED_BODY()

public:
	// DTO ??DataAsset Î≥Ä??(VO Î¨¥Í≤∞???†Ï?)
	virtual USkillDataAsset* MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO) = 0;
	
	// DataAsset ??DTO Î≥Ä??(?ÑÏöî???ÑÎìúÎß?Ï∂îÏ∂ú)
	virtual FSkillMasterDatabaseDTO MapDataAssetToDto(const USkillDataAsset* DataAsset) = 0;

	// Î¶¨ÏÜå??Í¥ÄÎ¶?
	virtual USkillDataAsset* LoadOrCreateDataAsset(int32 SkillId) = 0;
	virtual void CacheDataAsset(int32 SkillId, USkillDataAsset* DataAsset) = 0;
	virtual USkillDataAsset* GetCachedDataAsset(int32 SkillId) = 0;

	// Î∞∞Ïπò Î≥Ä??
	virtual TArray<USkillDataAsset*> MapDtosToDataAssets(const TArray<FSkillMasterDatabaseDTO>& DTOs) = 0;
	virtual TArray<FSkillMasterDatabaseDTO> MapDataAssetsToDtos(const TArray<USkillDataAsset*>& DataAssets) = 0;

	// Validation
	virtual bool ValidateDataAsset(const USkillDataAsset* DataAsset, FString& OutErrorMessage) = 0;
	virtual bool ValidateDto(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) = 0;

	// Asset Path Í¥ÄÎ¶?
	virtual FString GenerateAssetPath(int32 SkillId) = 0;
	virtual USkillDataAsset* LoadDataAssetFromPath(const FString& AssetPath) = 0;
};
