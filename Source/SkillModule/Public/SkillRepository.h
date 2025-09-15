
#pragma once

#include "CoreMinimal.h"
#include "../../GameSharedModule/Public/Interface/ISkillRepositoryInterface.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "SkillRepository.generated.h"

class USkillComponent;

UCLASS()
class SKILLMODULE_API USkillRepository : public UObject, public ISkillRepositoryInterface 
{
	GENERATED_BODY()

public:
	virtual void Initialize(IDBProviderInfra* Infra) override;
	
	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> LoadUserSkillSlots(const FString& UserId, const FString& SlotKey) override;

	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> SaveUserSkillSlots(const FString& UserId, const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs) override;

	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> LoadSkillMasterData(const TArray<int32>& SkillIds = {}) override;

	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime) override;

	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> ClearUserSkillSlots(const FString& UserId, const FString& SlotKey) override;

	virtual UE::Tasks::TTask<FSkillRepositoryResult3Layer> GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate) override;

private:
	// TObjectPtr<UDatabaseManager> DBManager;
	TSharedPtr<ISkillDBProvider> DBProvider;
};
