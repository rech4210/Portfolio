// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#include "Mappers/SkillAssetMapper.h"
#include "Data/SkillDataAsset.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

USkillAssetMapper::USkillAssetMapper()
{
	// ê¸°ë³¸ ê²½ë¡œ ?¤ì •
	SkillDataAssetBasePath = TEXT("/Game/Data/Skills/");
}

USkillDataAsset* USkillAssetMapper::MapDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO)
{
	// ê¸°ì¡´ ìºì‹œ??DataAsset ?•ì¸
	if (USkillDataAsset* CachedAsset = GetCachedDataAsset(DTO.SkillId))
	{
		// ìºì‹œ???ì…‹??DTOë¡??…ë°?´íŠ¸ (VO ë¬´ê²°??? ì??˜ë©´???„ìš”??ë¶€ë¶„ë§Œ)
		SyncDtoToDataAsset(DTO, CachedAsset);
		return CachedAsset;
	}

	// ?Œì¼?ì„œ ê¸°ì¡´ DataAsset ë¡œë“œ ?œë„
	FString AssetPath = GenerateAssetPath(DTO.SkillId);
	USkillDataAsset* ExistingAsset = LoadDataAssetFromPath(AssetPath);
	
	if (ExistingAsset)
	{
		// ê¸°ì¡´ ?ì…‹??DTOë¡??…ë°?´íŠ¸
		SyncDtoToDataAsset(DTO, ExistingAsset);
		CacheDataAsset(DTO.SkillId, ExistingAsset);
		return ExistingAsset;
	}

	// ?ˆë¡œ??DataAsset ?ì„± (ì£¼ì˜: VO êµ¬ì¡° ? ì?)
	USkillDataAsset* NewAsset = NewObject<USkillDataAsset>();
	if (!NewAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create SkillDataAsset for SkillId: %d"), DTO.SkillId);
		return nullptr;
	}

	// DTO?ì„œ DataAsset?¼ë¡œ ê¸°ë³¸ ?•ë³´ ë§¤í•‘ (VO ?„ë“œ??ë³´ì¡´)
	SyncDtoToDataAsset(DTO, NewAsset);

	// ìºì‹œ???€??
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
	
	// DataAsset?ì„œ DTOë¡??„ìš”???„ë“œë§?ì¶”ì¶œ
	return ExtractDtoFromDataAsset(DataAsset);
}

USkillDataAsset* USkillAssetMapper::LoadOrCreateDataAsset(int32 SkillId)
{
	// ìºì‹œ?ì„œ ë¨¼ì? ?•ì¸
	if (USkillDataAsset* CachedAsset = GetCachedDataAsset(SkillId))
	{
		return CachedAsset;
	}

	// ?Œì¼?ì„œ ë¡œë“œ ?œë„
	FString AssetPath = GenerateAssetPath(SkillId);
	USkillDataAsset* LoadedAsset = LoadDataAssetFromPath(AssetPath);
	
	if (LoadedAsset)
	{
		CacheDataAsset(SkillId, LoadedAsset);
		return LoadedAsset;
	}

	// ?ˆë¡œ??DataAsset ?ì„± (ìµœì†Œ?œì˜ ?•ë³´ë§??¤ì •)
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

	// DTO??ê¸°ë³¸ ?•ë³´ë§?DataAsset???™ê¸°??(VO ?„ë“œ?¤ì? ë³´ì¡´)
	DataAsset->SkillID = DTO.SkillId;
	DataAsset->CoolTime = DTO.BaseCooltime;
	DataAsset->CostAmount = DTO.BaseCost;
	
	// ì£¼ì˜: USkillDataAsset???¤ë¥¸ ?„ë“œ??GEClass, AbilityClass, TargetStrategyClass ???€
	// ?ë””?°ì—???¤ì •???ë³¸ ê°’ì„ ? ì??˜ê³  ê±´ë“œë¦¬ì? ?ŠìŒ
}

FSkillMasterDatabaseDTO USkillAssetMapper::ExtractDtoFromDataAsset(const USkillDataAsset* DataAsset)
{
	FSkillMasterDatabaseDTO DTO;
	
	if (!DataAsset)
	{
		return DTO;
	}
	
	// DataAsset?ì„œ ?°ì´?°ë² ?´ìŠ¤???€?¥í•  ?„ë“œë§?ì¶”ì¶œ
	DTO.SkillId = DataAsset->SkillID;
	DTO.BaseCooltime = DataAsset->CoolTime;
	DTO.BaseCost = DataAsset->CostAmount;
	DTO.bEnabled = true; // ê¸°ë³¸ê°?
	
	// DisplayNameê³?Description?€ DataAsset??ì§ì ‘?ì¸ ?€???„ë“œê°€ ?†ìœ¼ë¯€ë¡?
	// ê¸°ë³¸ê°??ëŠ” ?¤ë¥¸ ?ŒìŠ¤?ì„œ ê°€?¸ì?????
	DTO.DisplayName = FString::Printf(TEXT("Skill_%d"), DataAsset->SkillID);
	DTO.Description = TEXT("Generated from DataAsset");
	DTO.MaxLevel = 10; // ê¸°ë³¸ê°?(DataAsset??MaxLevel ?„ë“œê°€ ?†ìŒ)
	
	return DTO;
}
