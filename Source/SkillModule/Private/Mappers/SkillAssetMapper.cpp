// Fill out your copyright notice in the Description page of Project Settings.

#include "Mappers/SkillAssetMapper.h"
#include "Data/SkillDataAsset.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

USkillAssetMapper::USkillAssetMapper()
{
	// 기본 경로 설정
	SkillDataAssetBasePath = TEXT("/Game/Data/Skills/");
}

USkillDataAsset* USkillAssetMapper::MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO)
{
	// 기존 캐시된 DataAsset 확인
	if (USkillDataAsset* CachedAsset = GetCachedDataAsset(DTO.SkillId))
	{
		// 캐시된 에셋을 DTO로 업데이트 (VO 무결성 유지하면서 필요한 부분만)
		SyncDtoToDataAsset(DTO, CachedAsset);
		return CachedAsset;
	}

	// 파일에서 기존 DataAsset 로드 시도
	FString AssetPath = GenerateAssetPath(DTO.SkillId);
	USkillDataAsset* ExistingAsset = LoadDataAssetFromPath(AssetPath);
	
	if (ExistingAsset)
	{
		// 기존 에셋을 DTO로 업데이트
		SyncDtoToDataAsset(DTO, ExistingAsset);
		CacheDataAsset(DTO.SkillId, ExistingAsset);
		return ExistingAsset;
	}

	// 새로운 DataAsset 생성 (주의: VO 구조 유지)
	USkillDataAsset* NewAsset = NewObject<USkillDataAsset>();
	if (!NewAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create SkillDataAsset for SkillId: %d"), DTO.SkillId);
		return nullptr;
	}

	// DTO에서 DataAsset으로 기본 정보 매핑 (VO 필드는 보존)
	SyncDtoToDataAsset(DTO, NewAsset);

	// 캐시에 저장
	CacheDataAsset(DTO.SkillId, NewAsset);

	return NewAsset;
}

FSkillMasterDatabaseDTO USkillAssetMapper::MapDataAssetToDto(const USkillDataAsset* DataAsset)
{
	FSkillMasterDatabaseDTO DTO;
	
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("DataAsset is null in MapDataAssetToDto"));
		return DTO;
	}
	
	// DataAsset에서 DTO로 필요한 필드만 추출
	return ExtractDtoFromDataAsset(DataAsset);
}

USkillDataAsset* USkillAssetMapper::LoadOrCreateDataAsset(int32 SkillId)
{
	// 캐시에서 먼저 확인
	if (USkillDataAsset* CachedAsset = GetCachedDataAsset(SkillId))
	{
		return CachedAsset;
	}

	// 파일에서 로드 시도
	FString AssetPath = GenerateAssetPath(SkillId);
	USkillDataAsset* LoadedAsset = LoadDataAssetFromPath(AssetPath);
	
	if (LoadedAsset)
	{
		CacheDataAsset(SkillId, LoadedAsset);
		return LoadedAsset;
	}

	// 새로운 DataAsset 생성 (최소한의 정보만 설정)
	USkillDataAsset* NewAsset = NewObject<USkillDataAsset>();
	if (NewAsset)
	{
		NewAsset->SkillID = SkillId;
		CacheDataAsset(SkillId, NewAsset);
	}

	return NewAsset;
}

void USkillAssetMapper::CacheDataAsset(int32 SkillId, USkillDataAsset* DataAsset)
{
	if (DataAsset)
	{
		CachedDataAssets.Add(SkillId, DataAsset);
	}
}

USkillDataAsset* USkillAssetMapper::GetCachedDataAsset(int32 SkillId)
{
	if (USkillDataAsset** FoundAsset = CachedDataAssets.Find(SkillId))
	{
		return *FoundAsset;
	}
	return nullptr;
}

TArray<USkillDataAsset*> USkillAssetMapper::MapDtosToDataAssets(const TArray<FSkillMasterDatabaseDTO>& DTOs)
{
	TArray<USkillDataAsset*> DataAssets;
	DataAssets.Reserve(DTOs.Num());
	
	for (const FSkillMasterDatabaseDTO& DTO : DTOs)
	{
		if (USkillDataAsset* DataAsset = MapDtoToDataAsset(DTO))
		{
			DataAssets.Add(DataAsset);
		}
	}
	
	return DataAssets;
}

TArray<FSkillMasterDatabaseDTO> USkillAssetMapper::MapDataAssetsToDtos(const TArray<USkillDataAsset*>& DataAssets)
{
	TArray<FSkillMasterDatabaseDTO> DTOs;
	DTOs.Reserve(DataAssets.Num());
	
	for (const USkillDataAsset* DataAsset : DataAssets)
	{
		if (DataAsset)
		{
			DTOs.Add(MapDataAssetToDto(DataAsset));
		}
	}
	
	return DTOs;
}

bool USkillAssetMapper::ValidateDataAsset(const USkillDataAsset* DataAsset, FString& OutErrorMessage)
{
	if (!DataAsset)
	{
		OutErrorMessage = TEXT("DataAsset is null");
		return false;
	}
	
	if (DataAsset->SkillID <= 0)
	{
		OutErrorMessage = TEXT("Invalid SkillID: must be greater than 0");
		return false;
	}
	
	if (DataAsset->CoolTime < 0.0f)
	{
		OutErrorMessage = TEXT("Invalid CoolTime: must be greater than or equal to 0");
		return false;
	}
	
	if (DataAsset->CostAmount < 0.0f)
	{
		OutErrorMessage = TEXT("Invalid CostAmount: must be greater than or equal to 0");
		return false;
	}
	
	return true;
}

bool USkillAssetMapper::ValidateDto(const FSkillMasterDatabaseDTO& DTO, FString& OutErrorMessage)
{
	if (DTO.SkillId <= 0)
	{
		OutErrorMessage = TEXT("Invalid SkillId: must be greater than 0");
		return false;
	}
	
	if (DTO.DisplayName.IsEmpty())
	{
		OutErrorMessage = TEXT("Invalid DisplayName: cannot be empty");
		return false;
	}
	
	if (DTO.MaxLevel < 1)
	{
		OutErrorMessage = TEXT("Invalid MaxLevel: must be greater than or equal to 1");
		return false;
	}
	
	if (DTO.BaseCooltime < 0.0f)
	{
		OutErrorMessage = TEXT("Invalid BaseCooltime: must be greater than or equal to 0");
		return false;
	}
	
	if (DTO.BaseCost < 0.0f)
	{
		OutErrorMessage = TEXT("Invalid BaseCost: must be greater than or equal to 0");
		return false;
	}
	
	return true;
}

FString USkillAssetMapper::GenerateAssetPath(int32 SkillId)
{
	return FString::Printf(TEXT("%sSkill_%d.Skill_%d"), *SkillDataAssetBasePath, SkillId, SkillId);
}

USkillDataAsset* USkillAssetMapper::LoadDataAssetFromPath(const FString& AssetPath)
{
	if (AssetPath.IsEmpty())
	{
		return nullptr;
	}

	return LoadObject<USkillDataAsset>(nullptr, *AssetPath);
}

void USkillAssetMapper::SyncDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO, USkillDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		return;
	}

	// DTO의 기본 정보만 DataAsset에 동기화 (VO 필드들은 보존)
	DataAsset->SkillID = DTO.SkillId;
	DataAsset->CoolTime = DTO.BaseCooltime;
	DataAsset->CostAmount = DTO.BaseCost;
	
	// 주의: USkillDataAsset의 다른 필드들(GEClass, AbilityClass, TargetStrategyClass 등)은
	// 에디터에서 설정된 원본 값을 유지하고 건드리지 않음
}

FSkillMasterDatabaseDTO USkillAssetMapper::ExtractDtoFromDataAsset(const USkillDataAsset* DataAsset)
{
	FSkillMasterDatabaseDTO DTO;
	
	if (!DataAsset)
	{
		return DTO;
	}
	
	// DataAsset에서 데이터베이스에 저장할 필드만 추출
	DTO.SkillId = DataAsset->SkillID;
	DTO.BaseCooltime = DataAsset->CoolTime;
	DTO.BaseCost = DataAsset->CostAmount;
	DTO.bEnabled = true; // 기본값
	
	// DisplayName과 Description은 DataAsset에 직접적인 대응 필드가 없으므로
	// 기본값 또는 다른 소스에서 가져와야 함
	DTO.DisplayName = FString::Printf(TEXT("Skill_%d"), DataAsset->SkillID);
	DTO.Description = TEXT("Generated from DataAsset");
	DTO.MaxLevel = 10; // 기본값 (DataAsset에 MaxLevel 필드가 없음)
	
	return DTO;
}
