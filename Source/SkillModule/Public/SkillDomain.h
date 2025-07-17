// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "SkillDomain.generated.h"

class USkillDataAsset;

/**
 * DTO for skill slot data transfer
 */


/**
 * Pure domain object for skill configuration (aggregate root)
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

	const FSkillSlotDTO* GetSkillByGuid(const FGuid& SlotId) const
	{
		return SkillSlots.FindByPredicate([SlotId](const FSkillSlotDTO& Slot)
		{
			return Slot.SlotId == SlotId;
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
 * Repository result wrapper for async operations
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
