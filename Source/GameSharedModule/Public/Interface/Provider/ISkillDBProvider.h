// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameSharedModule/Public/DTO/SkillDTOs.h"
#include "Tasks/Task.h"
#include "ISkillDBProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class USkillDBProvider : public UInterface {
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API ISkillDBProvider {
	GENERATED_BODY()
public:
	// 3-layer architecture skill operations
	virtual UE::Tasks::TTask<TArray<FSkillSlotDatabaseDTO>> LoadUserSkillSlots(const FString& UserId, const FString& SlotKey) = 0;
	virtual UE::Tasks::TTask<bool> SaveUserSkillSlots(const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs) = 0;
	virtual UE::Tasks::TTask<TArray<FSkillMasterDatabaseDTO>> LoadSkillMasterData(const TArray<int32>& SkillIds) = 0;
	virtual UE::Tasks::TTask<bool> SaveSkillMasterData(const TArray<FSkillMasterDatabaseDTO>& SkillMasterDTOs) = 0;
	virtual UE::Tasks::TTask<bool> UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime) = 0;
	virtual UE::Tasks::TTask<bool> ClearUserSkillSlots(const FString& UserId, const FString& SlotKey) = 0;
	virtual UE::Tasks::TTask<TMap<int32, int32>> GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate) = 0;
};
