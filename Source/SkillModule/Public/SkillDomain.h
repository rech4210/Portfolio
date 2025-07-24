// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "SkillDomain.generated.h"

class USkillDataAsset;
//
// /**
//  * DEPRECATED: Legacy DTO structure for backward compatibility only
//  * Use FSkillSlotDatabaseDTO from DatabaseManager instead
//  */
// USTRUCT(BlueprintType)
// struct SKILLMODULE_API FSkillSlotDTO
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	FGuid SlotId;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	int32 SkillID = -1;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	int32 SlotIndex = -1;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	int32 SkillLevel = 1;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	FDateTime LastUsedTime;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	float RemainingCooldown = 0.0f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
// 	bool bIsActive = true;
//
// 	FSkillSlotDTO()
// 	{
// 		SlotId = FGuid::NewGuid();
// 		LastUsedTime = FDateTime::Now();
// 	}
// };

/**
 * 3-Layer Mapping Domain object for skill configuration (aggregate root)
 * Uses FSkillSlotDatabaseDTO for proper database integration
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillDomain3Layer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FString SlotKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<FSkillSlotDatabaseDTO> SkillSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<FSkillMasterDatabaseDTO> SkillMasterData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 MaxSlots = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FDateTime LastUpdateTime;

	FSkillDomain3Layer()
	{
		UserId = 0;
		SlotKey = TEXT("ActionBar");
		LastUpdateTime = FDateTime::Now();
	}

	FSkillDomain3Layer(int32 InUserId, const FString& InSlotKey, const TArray<FSkillSlotDatabaseDTO>& InSkillSlots)
		: UserId(InUserId), SlotKey(InSlotKey), SkillSlots(InSkillSlots)
	{
		LastUpdateTime = FDateTime::Now();
	}

	bool IsValid() const
	{
		return UserId > 0 && !SlotKey.IsEmpty() && SkillSlots.Num() <= MaxSlots;
	}

	bool HasSkill(int32 SkillId) const
	{
		return SkillSlots.ContainsByPredicate([SkillId](const FSkillSlotDatabaseDTO& Slot)
		{
			return Slot.SkillId == SkillId;
		});
	}

	bool HasSkillInSlot(int32 SlotIndex) const
	{
		return SkillSlots.ContainsByPredicate([SlotIndex](const FSkillSlotDatabaseDTO& Slot)
		{
			return Slot.SlotIndex == SlotIndex;
		});
	}

	const FSkillSlotDatabaseDTO* GetSkillBySlotIndex(int32 SlotIndex) const
	{
		return SkillSlots.FindByPredicate([SlotIndex](const FSkillSlotDatabaseDTO& Slot)
		{
			return Slot.SlotIndex == SlotIndex;
		});
	}

	int32 GetAvailableSlotIndex() const
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (!HasSkillInSlot(i))
			{
				return i;
			}
		}
		return -1; // No available slots
	}

	FSkillMasterDatabaseDTO* GetSkillMasterData(int32 SkillId)
	{
		return SkillMasterData.FindByPredicate([SkillId](const FSkillMasterDatabaseDTO& Master)
		{
			return Master.SkillId == SkillId;
		});
	}
};

/**
 * DEPRECATED: Legacy domain object for backward compatibility
 * Use FSkillDomain3Layer instead
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillDomain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FGuid PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<FSkillSlotDTO> SkillSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	int32 MaxSlots = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FDateTime LastUpdateTime;

	FSkillDomain()
	{
		LastUpdateTime = FDateTime::Now();
	}

	FSkillDomain(const FGuid& InPlayerId, const TArray<FSkillSlotDTO>& InSkillSlots)
		: PlayerId(InPlayerId), SkillSlots(InSkillSlots)
	{
		LastUpdateTime = FDateTime::Now();
	}

	bool IsValid() const
	{
		return PlayerId.IsValid() && SkillSlots.Num() <= MaxSlots;
	}

	bool HasSkill(int32 SkillID) const
	{
		return SkillSlots.ContainsByPredicate([SkillID](const FSkillSlotDTO& Slot)
		{
			return Slot.SkillID == SkillID && Slot.bIsActive;
		});
	}

	bool HasSkillInSlot(int32 SlotIndex) const
	{
		return SkillSlots.ContainsByPredicate([SlotIndex](const FSkillSlotDTO& Slot)
		{
			return Slot.SlotIndex == SlotIndex && Slot.bIsActive;
		});
	}

	const FSkillSlotDTO* GetSkillBySlotIndex(int32 SlotIndex) const
	{
		return SkillSlots.FindByPredicate([SlotIndex](const FSkillSlotDTO& Slot)
		{
			return Slot.SlotIndex == SlotIndex && Slot.bIsActive;
		});
	}

	const FSkillSlotDTO* GetSkillByIndex(int32 SlotIndex) const
	{
		return SkillSlots.FindByPredicate([SlotIndex](const FSkillSlotDTO& Slot)
		{
			return Slot.SlotIndex == SlotIndex;
		});
	}

	int32 GetAvailableSlotIndex() const
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (!HasSkillInSlot(i))
			{
				return i;
			}
		}
		return -1; // No available slots
	}
};

/**
 * 3-Layer Mapping Repository result wrapper for async operations
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillRepositoryResult3Layer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	FSkillDomain3Layer SkillData;

	// Helper data for different operations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	TArray<FSkillSlotDatabaseDTO> SkillSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	TArray<FSkillMasterDatabaseDTO> MasterData;

	static FSkillRepositoryResult3Layer Success(const FSkillDomain3Layer& InSkillData)
	{
		FSkillRepositoryResult3Layer Result;
		Result.bSuccess = true;
		Result.SkillData = InSkillData;
		return Result;
	}

	static FSkillRepositoryResult3Layer SuccessWithSlots(const TArray<FSkillSlotDatabaseDTO>& InSkillSlots)
	{
		FSkillRepositoryResult3Layer Result;
		Result.bSuccess = true;
		Result.SkillSlots = InSkillSlots;
		return Result;
	}

	static FSkillRepositoryResult3Layer SuccessWithMasterData(const TArray<FSkillMasterDatabaseDTO>& InMasterData)
	{
		FSkillRepositoryResult3Layer Result;
		Result.bSuccess = true;
		Result.MasterData = InMasterData;
		return Result;
	}

	static FSkillRepositoryResult3Layer Failure(const FString& InErrorMessage)
	{
		FSkillRepositoryResult3Layer Result;
		Result.bSuccess = false;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
};

/**
 * DEPRECATED: Legacy repository result wrapper for backward compatibility
 * Use FSkillRepositoryResult3Layer instead
 */
USTRUCT(BlueprintType)
struct SKILLMODULE_API FSkillRepositoryResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillRepositoryResult")
	FSkillDomain SkillData;

	static FSkillRepositoryResult Success(const FSkillDomain& InSkillData)
	{
		FSkillRepositoryResult Result;
		Result.bSuccess = true;
		Result.SkillData = InSkillData;
		return Result;
	}

	static FSkillRepositoryResult Failure(const FString& InErrorMessage)
	{
		FSkillRepositoryResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
};
