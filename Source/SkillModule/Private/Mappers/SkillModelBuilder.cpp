#include "Mappers/SkillModelBuilder.h"

#include "Components/SkillComponent.h"
#include "Data/SkillDataAsset.h"

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
	DomainModel.UserId = FCString::Atoi(*DatabaseDTO.UserId); // FString??int32�?변??
	DomainModel.SlotKey = DatabaseDTO.SlotKey;
	DomainModel.SlotIndex = DatabaseDTO.SlotIndex;
	DomainModel.SkillId = DatabaseDTO.SkillId;
	DomainModel.SkillLevel = DatabaseDTO.SkillLevel;
	DomainModel.LastUsedTime = DatabaseDTO.LastUsedTime;
	DomainModel.bIsActive = true; // DatabaseDTO??bIsActive ?�드가 ?�으므�?기본�??�용

	// 2. Business Logic Properties - RemainingCooldown 계산
	DomainModel.RemainingCooldown = CalculateRemainingCooldown(DatabaseDTO.LastUsedTime, SkillAsset->CoolTime);

	// 3. Business State Validation
	DomainModel.bCanUse = CanUseSkill(DomainModel.LastUsedTime, SkillAsset->CoolTime, 100, static_cast<int32>(SkillAsset->CostAmount));

	// 4. SkillDataAsset 참조 ?�??
	DomainModel.SkillDataAsset = SkillAsset;

	return DomainModel;
}

FSkillSlotReplicationData USkillModelBuilder::BuildSkillSlotData(const FSkillDomainModel& DomainModel)
{
	FSkillSlotReplicationData SlotData;
	
	// Domain Model에서 복제 데이터로 변환
	SlotData.SlotIndex = DomainModel.SlotIndex;
	SlotData.SkillId = DomainModel.SkillId;
	SlotData.SlotKey = DomainModel.SlotKey;
	SlotData.LastUsedTime = DomainModel.LastUsedTime;

	// SkillDataAsset이 있으면 기본 정보 추가
	if (DomainModel.SkillDataAsset)
	{
		SlotData.SkillName = DomainModel.SkillDataAsset->DisplayName.ToString();
		SlotData.SkillDescription = DomainModel.SkillDataAsset->Description.ToString();
		SlotData.Cooldown = DomainModel.SkillDataAsset->CoolTime;
	}
	else
	{
		SlotData.SkillName.Empty();
		SlotData.SkillDescription.Empty();
		SlotData.Cooldown = 0.0f;
	}

	return SlotData;
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
	// 1. Cooldown Check - ?�제 remaining cooldown 계산
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
// EXTRACTION METHODS (Entity ??DTO)
// ========================================================================

FSkillSlotDatabaseDTO USkillModelBuilder::ExtractSlotDTO(const FSkillSlotReplicationData& SlotData)
{
	FSkillSlotDatabaseDTO DTO;
	
	// FSkillSlotReplicationData에서 DTO로 변환
	DTO.SlotIndex = SlotData.SlotIndex;
	DTO.SkillId = SlotData.SkillId;
	DTO.SkillLevel = 1; // 기본값
	DTO.LastUsedTime = SlotData.LastUsedTime;
	DTO.SlotKey = SlotData.SlotKey;

	return DTO;
}

FSkillDomainModel USkillModelBuilder::ExtractDomainModel(const FSkillSlotReplicationData& SlotData)
{
	FSkillDomainModel DomainModel;
	
	// FSkillSlotReplicationData에서 DomainModel로 변환
	DomainModel.SlotIndex = SlotData.SlotIndex;
	DomainModel.SkillId = SlotData.SkillId;
	DomainModel.SkillLevel = 1; // 기본값
	DomainModel.LastUsedTime = SlotData.LastUsedTime;
	DomainModel.SlotKey = SlotData.SlotKey;
	DomainModel.bIsActive = !SlotData.IsEmpty();
	
	// Cooldown 계산 (복제 데이터에서 직접 사용)
	DomainModel.RemainingCooldown = CalculateRemainingCooldown(SlotData.LastUsedTime, SlotData.Cooldown);
	DomainModel.bCanUse = !IsOnCooldown(SlotData.LastUsedTime, SlotData.Cooldown);

	// SkillDataAsset은 LocalDataBaseLoader를 통해 재구성 필요
	DomainModel.SkillDataAsset = nullptr; // 복제 데이터에서는 직접 포함하지 않음

	return DomainModel;
}

// Helper methods for cooldown calculation
bool USkillModelBuilder::IsOnCooldown(const FDateTime& LastUsedTime, float BaseCooltime) const
{
	if (BaseCooltime <= 0.0f || LastUsedTime <= FDateTime::MinValue())
	{
		return false;
	}

	FDateTime Now = FDateTime::Now();
	double ElapsedSeconds = (Now - LastUsedTime).GetTotalSeconds();
	return ElapsedSeconds < BaseCooltime;
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
		// Find matching SkillDataAsset (?�드�??�정)
		USkillDataAsset* MatchingAsset = nullptr;
		for (USkillDataAsset* Asset : SkillAssets)
		{
			if (Asset && Asset->SkillID == DTO.SkillId) // SkillID ??SkillId
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
			UE_LOG(LogTemp, Warning, TEXT("SkillModelBuilder: No matching SkillDataAsset found for SkillId %d"), DTO.SkillId); // SkillID ??SkillId
		}
	}

	return DomainModels;
}

TArray<FSkillSlotReplicationData> USkillModelBuilder::BuildSkillSlotDataArray(const TArray<FSkillDomainModel>& DomainModels)
{
	TArray<FSkillSlotReplicationData> SkillSlotDataArray;
	
	for (const FSkillDomainModel& DomainModel : DomainModels)
	{
		FSkillSlotReplicationData SlotData = BuildSkillSlotData(DomainModel);
		SkillSlotDataArray.Add(SlotData);
	}

	return SkillSlotDataArray;
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

	if (DomainModel.SkillId <= 0) // SkillID ??SkillId
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

bool USkillModelBuilder::ValidateSkillSlotData(const FSkillSlotReplicationData& SlotData, FString& OutErrorMessage)
{
	if (SlotData.SlotIndex < 0)
	{
		OutErrorMessage = TEXT("Invalid SlotIndex in SkillSlotReplicationData");
		return false;
	}

	if (SlotData.SkillId <= 0)
	{
		OutErrorMessage = TEXT("Invalid SkillId in SkillSlotReplicationData");
		return false;
	}

	return true;
}
