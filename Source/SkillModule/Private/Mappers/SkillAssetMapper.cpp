
#include "Mappers/SkillAssetMapper.h"
#include "Data/SkillDataAsset.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

USkillAssetMapper::USkillAssetMapper()
{
	// 기본 경로 ?�정
	SkillDataAssetBasePath = TEXT("/Game/Data/Skills/");
}

USkillDataAsset* USkillAssetMapper::MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO)
{
	// 기존 캐시??DataAsset ?�인
	if (USkillDataAsset* CachedAsset = GetCachedDataAsset(DTO.SkillId))
	{
		// 캐시???�셋??DTO�??�데?�트 (VO 무결???��??�면???�요??부분만)
		SyncDtoToDataAsset(DTO, CachedAsset);
		return CachedAsset;
	}

	// ?�일?�서 기존 DataAsset 로드 ?�도
	FString AssetPath = GenerateAssetPath(DTO.SkillId);
	USkillDataAsset* ExistingAsset = LoadDataAssetFromPath(AssetPath);
	
	if (ExistingAsset)
	{
		// 기존 ?�셋??DTO�??�데?�트
		SyncDtoToDataAsset(DTO, ExistingAsset);
		CacheDataAsset(DTO.SkillId, ExistingAsset);
		return ExistingAsset;
	}

	// ?�로??DataAsset ?�성 (주의: VO 구조 ?��?)
	USkillDataAsset* NewAsset = NewObject<USkillDataAsset>();
	if (!NewAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create SkillDataAsset for SkillId: %d"), DTO.SkillId);
		return nullptr;
	}

	// DTO?�서 DataAsset?�로 기본 ?�보 매핑 (VO ?�드??보존)
	SyncDtoToDataAsset(DTO, NewAsset);

	// 캐시???�??
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
	
	// DataAsset?�서 DTO�??�요???�드�?추출
	return ExtractDtoFromDataAsset(DataAsset);
}

USkillDataAsset* USkillAssetMapper::LoadOrCreateDataAsset(int32 SkillId)
{
	// 캐시?�서 먼�? ?�인
	if (USkillDataAsset* CachedAsset = GetCachedDataAsset(SkillId))
	{
		return CachedAsset;
	}

	// ?�일?�서 로드 ?�도
	FString AssetPath = GenerateAssetPath(SkillId);
	USkillDataAsset* LoadedAsset = LoadDataAssetFromPath(AssetPath);
	
	if (LoadedAsset)
	{
		CacheDataAsset(SkillId, LoadedAsset);
		return LoadedAsset;
	}

	// ?�로??DataAsset ?�성 (최소?�의 ?�보�??�정)
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

	// DTO??기본 ?�보�?DataAsset???�기??(VO ?�드?��? 보존)
	DataAsset->SkillID = DTO.SkillId;
	DataAsset->CoolTime = DTO.BaseCooltime;
	DataAsset->CostAmount = DTO.BaseCost;
	
	// 주의: USkillDataAsset???�른 ?�드??GEClass, AbilityClass, TargetStrategyClass ???�
	// ?�디?�에???�정???�본 값을 ?��??�고 건드리�? ?�음
}

FSkillMasterDatabaseDTO USkillAssetMapper::ExtractDtoFromDataAsset(const USkillDataAsset* DataAsset)
{
	FSkillMasterDatabaseDTO DTO;
	
	if (!DataAsset)
	{
		return DTO;
	}
	
	// DataAsset?�서 ?�이?�베?�스???�?�할 ?�드�?추출
	DTO.SkillId = DataAsset->SkillID;
	DTO.BaseCooltime = DataAsset->CoolTime;
	DTO.BaseCost = DataAsset->CostAmount;
	DTO.bEnabled = true; // 기본�?
	
	// DisplayName�?Description?� DataAsset??직접?�인 ?�???�드가 ?�으므�?
	// 기본�??�는 ?�른 ?�스?�서 가?��?????
	DTO.DisplayName = FString::Printf(TEXT("Skill_%d"), DataAsset->SkillID);
	DTO.Description = TEXT("Generated from DataAsset");
	DTO.MaxLevel = 10; // 기본�?(DataAsset??MaxLevel ?�드가 ?�음)
	
	return DTO;
}
