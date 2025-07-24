// Fill out your copyright notice in the Description page of Project Settings.

#include "Mappers/SkillDtoMapper.h"
#include "Engine/Engine.h"

FSkillSlotDatabaseDTO USkillDtoMapper::MapSlotFromSqlResult(const TMap<FString, FString>& SqlRow)
{
	FSkillSlotDatabaseDTO DTO;
	
	// SQL 결과?�서 DTO�?매핑
	if (SqlRow.Contains(TEXT("user_id")))
	{
		DTO.UserId = FCString::Atoi(*SqlRow[TEXT("user_id")]);
	}
	
	if (SqlRow.Contains(TEXT("slot_key")))
	{
		DTO.SlotKey = SqlRow[TEXT("slot_key")];
	}
	
	if (SqlRow.Contains(TEXT("skill_id")))
	{
		DTO.SkillId = FCString::Atoi(*SqlRow[TEXT("skill_id")]);
	}
	
	if (SqlRow.Contains(TEXT("slot_index")))
	{
		DTO.SlotIndex = FCString::Atoi(*SqlRow[TEXT("slot_index")]);
	}
	
	if (SqlRow.Contains(TEXT("skill_level")))
	{
		DTO.SkillLevel = FCString::Atoi(*SqlRow[TEXT("skill_level")]);
	}
	
	if (SqlRow.Contains(TEXT("last_used_time")))
	{
		FString TimeString = SqlRow[TEXT("last_used_time")];
		if (!TimeString.IsEmpty() && TimeString != TEXT("NULL"))
		{
			FDateTime::ParseIso8601(*TimeString, DTO.LastUsedTime);
		}
	}
	
	return DTO;
}

FSkillMasterDatabaseDTO USkillDtoMapper::MapMasterFromSqlResult(const TMap<FString, FString>& SqlRow)
{
	FSkillMasterDatabaseDTO DTO;
	
	if (SqlRow.Contains(TEXT("skill_id")))
	{
		DTO.SkillId = FCString::Atoi(*SqlRow[TEXT("skill_id")]);
	}
	
	if (SqlRow.Contains(TEXT("display_name")))
	{
		DTO.DisplayName = SqlRow[TEXT("display_name")];
	}
	
	if (SqlRow.Contains(TEXT("description")))
	{
		DTO.Description = SqlRow[TEXT("description")];
	}
	
	if (SqlRow.Contains(TEXT("max_level")))
	{
		DTO.MaxLevel = FCString::Atoi(*SqlRow[TEXT("max_level")]);
	}
	
	if (SqlRow.Contains(TEXT("base_cooltime")))
	{
		DTO.BaseCooltime = FCString::Atof(*SqlRow[TEXT("base_cooltime")]);
	}
	
	if (SqlRow.Contains(TEXT("base_cost")))
	{
		DTO.BaseCost = FCString::Atof(*SqlRow[TEXT("base_cost")]);
	}
	
	if (SqlRow.Contains(TEXT("enabled")))
	{
		DTO.bEnabled = SqlRow[TEXT("enabled")] == TEXT("1") || SqlRow[TEXT("enabled")].ToLower() == TEXT("true");
	}
	
	if (SqlRow.Contains(TEXT("description")))
	{
		DTO.Description = SqlRow[TEXT("description")];
	}
	
	return DTO;
}

TMap<FString, FString> USkillDtoMapper::MapSlotToSqlParams(const FSkillSlotDatabaseDTO& DTO)
{
	TMap<FString, FString> Params;
	
	Params.Add(TEXT("user_id"), FString::FromInt(DTO.UserId));
	Params.Add(TEXT("slot_key"), DTO.SlotKey);
	Params.Add(TEXT("skill_id"), FString::FromInt(DTO.SkillId));
	Params.Add(TEXT("slot_index"), FString::FromInt(DTO.SlotIndex));
	Params.Add(TEXT("skill_level"), FString::FromInt(DTO.SkillLevel));
	
	if (DTO.LastUsedTime != FDateTime::MinValue())
	{
		Params.Add(TEXT("last_used_time"), DTO.LastUsedTime.ToIso8601());
	}
	else
	{
		Params.Add(TEXT("last_used_time"), TEXT("NULL"));
	}
	
	return Params;
}

TMap<FString, FString> USkillDtoMapper::MapMasterToSqlParams(const FSkillMasterDatabaseDTO& DTO)
{
	TMap<FString, FString> Params;
	
	Params.Add(TEXT("skill_id"), FString::FromInt(DTO.SkillId));
	Params.Add(TEXT("display_name"), DTO.DisplayName);
	Params.Add(TEXT("max_level"), FString::FromInt(DTO.MaxLevel));
	Params.Add(TEXT("base_cooltime"), FString::SanitizeFloat(DTO.BaseCooltime));
	Params.Add(TEXT("base_cost"), FString::FromInt(DTO.BaseCost));
	Params.Add(TEXT("base_damage"), FString::SanitizeFloat(DTO.BaseCost));
	Params.Add(TEXT("description"), DTO.Description);
	Params.Add(TEXT("description"), DTO.Description);
	
	return Params;
}

bool USkillDtoMapper::ValidateSlotDTO(const FSkillSlotDatabaseDTO& DTO, FString& OutErrorMessage)
{
	if (DTO.UserId <= 0)
	{
		OutErrorMessage = TEXT("Invalid UserId: must be greater than 0");
		return false;
	}
	
	if (DTO.SlotKey.IsEmpty())
	{
		OutErrorMessage = TEXT("Invalid SlotKey: cannot be empty");
		return false;
	}
	
	if (DTO.SlotIndex < 0)
	{
		OutErrorMessage = TEXT("Invalid SlotIndex: must be greater than or equal to 0");
		return false;
	}
	
	if (DTO.SkillId < 0)
	{
		OutErrorMessage = TEXT("Invalid SkillId: must be greater than or equal to 0");
		return false;
	}
	
	if (DTO.SkillLevel < 1)
	{
		OutErrorMessage = TEXT("Invalid SkillLevel: must be greater than or equal to 1");
		return false;
	}
	
	return true;
}

bool USkillDtoMapper::ValidateMasterDTO(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage)
{
	if (DTO.SkillId <= 0)
	{
		OutErrorMessage = TEXT("Invalid SkillId: must be greater than 0");
		return false;
	}
	
	if (DTO.DisplayName.IsEmpty())
	{
		OutErrorMessage = TEXT("Invalid SkillName: cannot be empty");
		return false;
	}
	
	if (DTO.MaxLevel < 1)
	{
		OutErrorMessage = TEXT("Invalid MaxLevel: must be greater than or equal to 1");
		return false;
	}
	
	if (DTO.BaseCooltime < 0.0f)
	{
		OutErrorMessage = TEXT("Invalid BaseCooldown: must be greater than or equal to 0");
		return false;
	}
	
	if (DTO.BaseCost < 0)
	{
		OutErrorMessage = TEXT("Invalid BaseManaCost: must be greater than or equal to 0");
		return false;
	}
	
	return true;
}

TArray<FSkillSlotDatabaseDTO> USkillDtoMapper::MapSlotsFromSqlResults(const TArray<TMap<FString, FString>>& SqlRows)
{
	TArray<FSkillSlotDatabaseDTO> DTOs;
	DTOs.Reserve(SqlRows.Num());
	
	for (const auto& Row : SqlRows)
	{
		DTOs.Add(MapSlotFromSqlResult(Row));
	}
	
	return DTOs;
}

TArray<FSkillMasterDatabaseDTO> USkillDtoMapper::MapMastersFromSqlResults(const TArray<TMap<FString, FString>>& SqlRows)
{
	TArray<FSkillMasterDatabaseDTO> DTOs;
	DTOs.Reserve(SqlRows.Num());
	
	for (const auto& Row : SqlRows)
	{
		DTOs.Add(MapMasterFromSqlResult(Row));
	}
	
	return DTOs;
}
