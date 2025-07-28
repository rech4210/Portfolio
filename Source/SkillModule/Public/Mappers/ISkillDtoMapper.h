// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "ISkillDtoMapper.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class SKILLMODULE_API USkillDtoMapperInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * SQL ??DTO 매핑 ?�터?�이??
 * ?�이?�베?�스?� DTO 구조�?간의 변?�을 ?�당
 */
class SKILLMODULE_API ISkillDtoMapperInterface
{
	GENERATED_BODY()

public:
	// SQL ResultSet ??DTO 변??
	virtual FSkillSlotDatabaseDTO MapSlotFromSqlResult(const TMap<FString, FString>& SqlRow) = 0;
	virtual FSkillMasterDatabaseDTO MapMasterFromSqlResult(const TMap<FString, FString>& SqlRow) = 0;

	// DTO ??SQL Parameters 변??
	virtual TMap<FString, FString> MapSlotToSqlParams(const FSkillSlotDatabaseDTO& DTO) = 0;
	virtual TMap<FString, FString> MapMasterToSqlParams(const FSkillMasterDatabaseDTO& DTO) = 0;

	// Validation
	virtual bool ValidateSlotDTO(const FSkillSlotDatabaseDTO& DTO, FString& OutErrorMessage) = 0;
	virtual bool ValidateMasterDTO(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) = 0;

	// 배치 변??
	virtual TArray<FSkillSlotDatabaseDTO> MapSlotsFromSqlResults(const TArray<TMap<FString, FString>>& SqlRows) = 0;
	virtual TArray<FSkillMasterDatabaseDTO> MapMastersFromSqlResults(const TArray<TMap<FString, FString>>& SqlRows) = 0;
};
