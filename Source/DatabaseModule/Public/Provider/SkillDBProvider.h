#pragma once
#include "Interface/Provider/ISkillDBProvider.h"
#include "DatabaseManager.h"

class SkillDBProvider : public ISkillDBProvider{
public:
	SkillDBProvider(UDatabaseManager * InDBManager): DBManager(InDBManager) {}

public:
	// 3-layer operations
	virtual UE::Tasks::TTask<TArray<FSkillSlotDatabaseDTO>> LoadUserSkillSlots(const FString& UserId, const FString& SlotKey) override;
	virtual UE::Tasks::TTask<bool> SaveUserSkillSlots(const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs) override;
	virtual UE::Tasks::TTask<TArray<FSkillMasterDatabaseDTO>> LoadSkillMasterData(const TArray<int32>& SkillIds) override;
	virtual UE::Tasks::TTask<bool> SaveSkillMasterData(const TArray<FSkillMasterDatabaseDTO>& SkillMasterDTOs) override;
	virtual UE::Tasks::TTask<bool> UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime) override;
	virtual UE::Tasks::TTask<bool> ClearUserSkillSlots(const FString& UserId, const FString& SlotKey) override;
	virtual UE::Tasks::TTask<TMap<int32, int32>> GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate) override;
private:
	/*DB APIs from interface*/
	TWeakObjectPtr<UDatabaseManager> DBManager;
};
