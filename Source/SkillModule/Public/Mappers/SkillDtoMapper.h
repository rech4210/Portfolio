
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Mappers/ISkillDtoMapper.h"
#include "SkillDtoMapper.generated.h"

/**
 * SQL ??DTO 매핑 구현�?
 * ?�이?�베?�스?� DTO 구조�?간의 변?�을 구현
 */
UCLASS(BlueprintType)
class SKILLMODULE_API USkillDtoMapper : public UObject, public ISkillDtoMapperInterface
{
	GENERATED_BODY()

public:
	USkillDtoMapper() = default;

	// ISkillDtoMapperInterface interface
	virtual FSkillSlotDatabaseDTO MapSlotFromSqlResult(const TMap<FString, FString>& SqlRow) override;
	virtual FSkillMasterDatabaseDTO MapMasterFromSqlResult(const TMap<FString, FString>& SqlRow) override;
	virtual TMap<FString, FString> MapSlotToSqlParams(const FSkillSlotDatabaseDTO& DTO) override;
	virtual TMap<FString, FString> MapMasterToSqlParams(const FSkillMasterDatabaseDTO& DTO) override;
	virtual bool ValidateSlotDTO(const FSkillSlotDatabaseDTO& DTO, FString& OutErrorMessage) override;
	virtual bool ValidateMasterDTO(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage) override;
	virtual TArray<FSkillSlotDatabaseDTO> MapSlotsFromSqlResults(const TArray<TMap<FString, FString>>& SqlRows) override;
	virtual TArray<FSkillMasterDatabaseDTO> MapMastersFromSqlResults(const TArray<TMap<FString, FString>>& SqlRows) override;
};
