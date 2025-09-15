#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SkillDTOs.generated.h"

// Extracted Skill-related DTO structs from DatabaseManager.h to decouple DB layer.

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FSkillSlotDatabaseDTO
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString UserId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString SlotKey;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SkillId = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SlotIndex = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SkillLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FDateTime LastUsedTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FDateTime CreatedAt;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FDateTime UpdatedAt;

	FSkillSlotDatabaseDTO()
		: UserId(TEXT("")), SkillId(0), SlotIndex(-1), SkillLevel(1)
	{
		LastUsedTime = FDateTime::MinValue();
		CreatedAt = FDateTime::Now();
		UpdatedAt = FDateTime::Now();
	}

	bool IsValid() const { return !UserId.IsEmpty() && !SlotKey.IsEmpty() && SlotIndex >= 0; }
};

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FSkillMasterDatabaseDTO
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SkillId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	float BaseCooltime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	float BaseCost = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 MaxLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	bool bEnabled = true;

	FSkillMasterDatabaseDTO(): SkillId(0), BaseCooltime(0.0f), BaseCost(0.0f), MaxLevel(1), bEnabled(true) {}
};

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FSkillSlotDTO
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 SkillID;
	UPROPERTY() int32 SlotIndex;
	UPROPERTY() FDateTime LastUsedTime;
	UPROPERTY() float RemainingCooldown;
	UPROPERTY() bool bIsActive;
	UPROPERTY() FString SkillData;

	FSkillSlotDTO(): SkillID(0), SlotIndex(-1), RemainingCooldown(0.0f), bIsActive(true) { LastUsedTime = FDateTime::Now(); }
	FSkillSlotDTO(int32 InSkillID, int32 InSlotIndex): SkillID(InSkillID), SlotIndex(InSlotIndex), RemainingCooldown(0.0f), bIsActive(true) { LastUsedTime = FDateTime::Now(); }
};
