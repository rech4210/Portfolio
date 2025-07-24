// Fill out your copyright notice in the Description page of Project Settings.

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
 * DTO ↔ DataAsset 매핑 인터페이스
 * DTO와 UE5 DataAsset 간의 변환을 담당
 * 주의: USkillDataAsset은 VO(Value Object)이므로 원본 구조를 유지해야 함
 */
class SKILLMODULE_API ISkillAssetMapperInterface
{
	GENERATED_BODY()

public:
	// DTO → DataAsset 변환 (VO 무결성 유지)
	virtual USkillDataAsset* MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO) = 0;
	
	// DataAsset → DTO 변환 (필요한 필드만 추출)
	virtual FSkillMasterDatabaseDTO MapDataAssetToDto(const USkillDataAsset* DataAsset) = 0;

	// 리소스 관리
	virtual USkillDataAsset* LoadOrCreateDataAsset(int32 SkillId) = 0;
	virtual void CacheDataAsset(int32 SkillId, USkillDataAsset* DataAsset) = 0;
	virtual USkillDataAsset* GetCachedDataAsset(int32 SkillId) = 0;

	// 배치 변환
	virtual TArray<USkillDataAsset*> MapDtosToDataAssets(const TArray<FSkillMasterDatabaseDTO>& DTOs) = 0;
	virtual TArray<FSkillMasterDatabaseDTO> MapDataAssetsToDtos(const TArray<USkillDataAsset*>& DataAssets) = 0;

	// Validation
	virtual bool ValidateDataAsset(const USkillDataAsset* DataAsset, FString& OutErrorMessage) = 0;
	virtual bool ValidateDto(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) = 0;

	// Asset Path 관리
	virtual FString GenerateAssetPath(int32 SkillId) = 0;
	virtual USkillDataAsset* LoadDataAssetFromPath(const FString& AssetPath) = 0;
};
