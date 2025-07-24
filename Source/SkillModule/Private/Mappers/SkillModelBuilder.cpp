// Fill out your copyright notice in the Description page of Project Settings.

#include "Mappers/SkillModelBuilder.h"
#include "Entities/SkillSlot.h"
#include "Data/SkillDataAsset.h"
#include "DatabaseModule/Public/DatabaseManager.h"

USkillModelBuilder::USkillModelBuilder()
{
	// UObject constructor - no PrimaryComponentTick
}

// ========================================================================
// CORE DOMAIN MODEL BUILDING METHODS
// ========================================================================

FSkillDomainModel USkillModelBuilder::BuildDomainModel(const FSkillSlotDatabaseDTO& DatabaseDTO, USkillDataAsset* SkillAsset)
{
	FSkillDomainModel DomainModel;
	
	if (!SkillAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillModelBuilder: SkillAsset is null"));
		return DomainModel;
	}

	// 1. Core Properties from Database DTO
	DomainModel.UserId = DatabaseDTO.UserId;
	DomainModel.SlotKey = DatabaseDTO.SlotKey;
	DomainModel.SlotIndex = DatabaseDTO.SlotIndex;
	DomainModel.SkillId = DatabaseDTO.SkillId;
	DomainModel.SkillLevel = DatabaseDTO.SkillLevel;
	DomainModel.LastUsedTime = DatabaseDTO.LastUsedTime;
	DomainModel.bIsActive = true; // DatabaseDTO에 bIsActive 필드가 없으므로 기본값 사용

	// 2. Business Logic Properties - RemainingCooldown 계산
	DomainModel.RemainingCooldown = CalculateRemainingCooldown(DatabaseDTO.LastUsedTime, SkillAsset->CoolTime);

	// 3. Business State Validation
	DomainModel.bCanUse = CanUseSkill(DomainModel.LastUsedTime, SkillAsset->CoolTime, 100, static_cast<int32>(SkillAsset->CostAmount));

	// 4. SkillDataAsset 참조 저장
	DomainModel.SkillDataAsset = SkillAsset;

	return DomainModel;
}

USkillSlot* USkillModelBuilder::BuildSkillSlotEntity(const FSkillDomainModel& DomainModel)
{
	USkillSlot* SkillSlot = NewObject<USkillSlot>();
	
	if (!SkillSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillModelBuilder: Failed to create USkillSlot"));
		return nullptr;
	}

	// Domain Model → Entity 변환 (USkillSlot의 public 필드 직접 설정)
	SkillSlot->SlotIndex = DomainModel.SlotIndex;
	SkillSlot->SkillId = DomainModel.SkillId;
	SkillSlot->SkillData = DomainModel.SkillDataAsset;
	SkillSlot->LastUsedTime = DomainModel.LastUsedTime;
	SkillSlot->SlotKey = DomainModel.SlotKey;

	// Initialize 메서드 호출
	SkillSlot->Initialize(DomainModel.SlotIndex, DomainModel.SlotKey, DomainModel.SkillDataAsset);
	
	// SetSkillData 메서드 호출 (SkillId 설정)
	if (DomainModel.SkillDataAsset)
	{
		SkillSlot->SetSkillData(DomainModel.SkillDataAsset, DomainModel.SkillId);
	}

	return SkillSlot;
}

// ========================================================================
// BUSINESS LOGIC CALCULATIONS
// ========================================================================

float USkillModelBuilder::CalculateRemainingCooldown(const FDateTime& LastUsedTime, float StoredCooldown)
{
	if (LastUsedTime == FDateTime(0))
	{
		return 0.0f; // Never used
	}

	FDateTime CurrentTime = FDateTime::Now();
	float TimePassed = (CurrentTime - LastUsedTime).GetTotalSeconds();
	
	float RemainingTime = FMath::Max(0.0f, StoredCooldown - TimePassed);
	return RemainingTime;
}

bool USkillModelBuilder::CanUseSkill(const FDateTime& LastUsedTime, float BaseCooldown, int32 CurrentMana, int32 RequiredMana)
{
	// 1. Cooldown Check - 실제 remaining cooldown 계산
	float RemainingCooldown = CalculateRemainingCooldown(LastUsedTime, BaseCooldown);
	if (RemainingCooldown > 0.0f)
	{
		return false;
	}

	// 2. Mana Check
	if (CurrentMana < RequiredMana)
	{
		return false;
	}

	return true;
}

int32 USkillModelBuilder::CalculateScaledValue(int32 BaseValue, int32 Level)
{
	// Simple scaling formula: BaseValue + (Level - 1) * 10%
	float ScaleFactor = 1.0f + ((Level - 1) * 0.1f);
	return FMath::RoundToInt(BaseValue * ScaleFactor);
}

float USkillModelBuilder::CalculateScaledValue(float BaseValue, int32 Level)
{
	// Simple scaling formula for float values
	float ScaleFactor = 1.0f + ((Level - 1) * 0.1f);
	return BaseValue * ScaleFactor;
}

// ========================================================================
// EXTRACTION METHODS (Entity → DTO)
// ========================================================================

FSkillSlotDatabaseDTO USkillModelBuilder::ExtractSlotDTO(const USkillSlot* SkillSlot)
{
	FSkillSlotDatabaseDTO DTO;
	
	if (!SkillSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillModelBuilder: SkillSlot is null"));
		return DTO;
	}

	// USkillSlot의 실제 public 필드 사용
	DTO.SlotIndex = SkillSlot->SlotIndex;
	DTO.SkillId = SkillSlot->SkillId; // SkillID → SkillId
	DTO.SkillLevel = 1; // SkillLevel 필드가 없으므로 기본값
	DTO.LastUsedTime = SkillSlot->LastUsedTime;
	DTO.SlotKey = SkillSlot->SlotKey;

	return DTO;
}

FSkillDomainModel USkillModelBuilder::ExtractDomainModel(const USkillSlot* SkillSlot)
{
	FSkillDomainModel DomainModel;
	
	if (!SkillSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillModelBuilder: SkillSlot is null"));
		return DomainModel;
	}

	// USkillSlot의 실제 public 필드 사용
	DomainModel.SlotIndex = SkillSlot->SlotIndex;
	DomainModel.SkillId = SkillSlot->SkillId; // SkillID → SkillId
	DomainModel.SkillLevel = 1; // 기본값
	DomainModel.LastUsedTime = SkillSlot->LastUsedTime;
	DomainModel.SlotKey = SkillSlot->SlotKey;
	DomainModel.bIsActive = !SkillSlot->IsEmpty(); // IsActive 메서드가 없으므로 IsEmpty()의 반대값 사용
	DomainModel.SkillDataAsset = SkillSlot->SkillData;
	
	// Cooldown 계산
	if (SkillSlot->SkillData)
	{
		DomainModel.RemainingCooldown = SkillSlot->GetRemainingCooldown(SkillSlot->SkillData->CoolTime);
		DomainModel.bCanUse = !SkillSlot->IsOnCooldown(SkillSlot->SkillData->CoolTime);
	}

	return DomainModel;
}

// ========================================================================
// BATCH PROCESSING METHODS
// ========================================================================

TArray<FSkillDomainModel> USkillModelBuilder::BuildDomainModels(const TArray<FSkillSlotDatabaseDTO>& DatabaseDTOs, 
																const TArray<USkillDataAsset*>& SkillAssets)
{
	TArray<FSkillDomainModel> DomainModels;
	
	for (const FSkillSlotDatabaseDTO& DTO : DatabaseDTOs)
	{
		// Find matching SkillDataAsset (필드명 수정)
		USkillDataAsset* MatchingAsset = nullptr;
		for (USkillDataAsset* Asset : SkillAssets)
		{
			if (Asset && Asset->SkillID == DTO.SkillId) // SkillID → SkillId
			{
				MatchingAsset = Asset;
				break;
			}
		}

		if (MatchingAsset)
		{
			FSkillDomainModel DomainModel = BuildDomainModel(DTO, MatchingAsset);
			DomainModels.Add(DomainModel);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillModelBuilder: No matching SkillDataAsset found for SkillId %d"), DTO.SkillId); // SkillID → SkillId
		}
	}

	return DomainModels;
}

TArray<USkillSlot*> USkillModelBuilder::BuildSkillSlotEntities(const TArray<FSkillDomainModel>& DomainModels)
{
	TArray<USkillSlot*> SkillSlots;
	
	for (const FSkillDomainModel& DomainModel : DomainModels)
	{
		USkillSlot* SkillSlot = BuildSkillSlotEntity(DomainModel);
		if (SkillSlot)
		{
			SkillSlots.Add(SkillSlot);
		}
	}

	return SkillSlots;
}

// ========================================================================
// VALIDATION METHODS
// ========================================================================

bool USkillModelBuilder::ValidateDomainModel(const FSkillDomainModel& DomainModel, FString& OutErrorMessage)
{
	if (DomainModel.SlotIndex < 0)
	{
		OutErrorMessage = TEXT("Invalid SlotIndex: must be >= 0");
		return false;
	}

	if (DomainModel.SkillId <= 0) // SkillID → SkillId
	{
		OutErrorMessage = TEXT("Invalid SkillId: must be > 0");
		return false;
	}

	if (DomainModel.SkillLevel <= 0)
	{
		OutErrorMessage = TEXT("Invalid SkillLevel: must be > 0");
		return false;
	}

	return true;
}

bool USkillModelBuilder::ValidateSkillSlotEntity(const USkillSlot* SkillSlot, FString& OutErrorMessage)
{
	if (!SkillSlot)
	{
		OutErrorMessage = TEXT("SkillSlot is null");
		return false;
	}

	if (SkillSlot->SlotIndex < 0) // GetSlotIndex() → SlotIndex
	{
		OutErrorMessage = TEXT("Invalid SlotIndex in SkillSlot");
		return false;
	}

	if (SkillSlot->SkillId <= 0) // GetSkillID() → SkillId
	{
		OutErrorMessage = TEXT("Invalid SkillId in SkillSlot");
		return false;
	}

	return true;
}
