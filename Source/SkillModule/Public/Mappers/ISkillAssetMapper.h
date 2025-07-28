
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
 * DTO ??DataAsset 매핑 ?�터?�이??
 * DTO?� UE5 DataAsset 간의 변?�을 ?�당
 * 주의: USkillDataAsset?� VO(Value Object)?��?�??�본 구조�??��??�야 ??
 */
class SKILLMODULE_API ISkillAssetMapperInterface
{
	GENERATED_BODY()

public:
	// DTO ??DataAsset 변??(VO 무결???��?)
	virtual USkillDataAsset* MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO) = 0;
	
	// DataAsset ??DTO 변??(?�요???�드�?추출)
	virtual FSkillMasterDatabaseDTO MapDataAssetToDto(const USkillDataAsset* DataAsset) = 0;

	// 리소??관�?
	virtual USkillDataAsset* LoadOrCreateDataAsset(int32 SkillId) = 0;
	virtual void CacheDataAsset(int32 SkillId, USkillDataAsset* DataAsset) = 0;
	virtual USkillDataAsset* GetCachedDataAsset(int32 SkillId) = 0;

	// 배치 변??
	virtual TArray<USkillDataAsset*> MapDtosToDataAssets(const TArray<FSkillMasterDatabaseDTO>& DTOs) = 0;
	virtual TArray<FSkillMasterDatabaseDTO> MapDataAssetsToDtos(const TArray<USkillDataAsset*>& DataAssets) = 0;

	// Validation
	virtual bool ValidateDataAsset(const USkillDataAsset* DataAsset, FString& OutErrorMessage) = 0;
	virtual bool ValidateDto(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) = 0;

	// Asset Path 관�?
	virtual FString GenerateAssetPath(int32 SkillId) = 0;
	virtual USkillDataAsset* LoadDataAssetFromPath(const FString& AssetPath) = 0;
};
