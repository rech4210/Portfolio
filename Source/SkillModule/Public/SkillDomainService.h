#pragma once

#include "CoreMinimal.h"
#include "SkillDomain.h"
#include "GameSharedModule/Public/DTO/SkillDTOs.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "Interface/PlayerIdentityInterface.h"
#include "SkillDomainService.generated.h"

class USkillComponent;
class ISkillRepositoryInterface;
class USkillDataAsset;
struct FSkillSlotDTO;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillOperationSucceeded, const FGuid&, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillOperationFailed, const FGuid&, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillLoadCompleted, const FGuid&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillSaveCompleted, const FGuid&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillDomainRegistered, const FGuid&, const FSkillSlotDTO&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillDomainUnregistered, const FGuid&, const FGuid&);

UCLASS(BlueprintType)
class SKILLMODULE_API USkillDomainService : public UObject
{
	GENERATED_BODY()

public:
	USkillDomainService();

	void Initialize(TScriptInterface<ISkillRepositoryInterface> Repository);

	void LoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	void SavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	void UpdatePlayerSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData);

	void UpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime);

	void ClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	// Domain Events
	FOnSkillOperationSucceeded OnSkillOperationSucceeded;
	FOnSkillOperationFailed OnSkillOperationFailed;
	FOnSkillLoadCompleted OnSkillLoadCompleted;
	FOnSkillSaveCompleted OnSkillSaveCompleted;
	FOnSkillDomainRegistered OnSkillDomainRegistered;
	FOnSkillDomainUnregistered OnSkillDomainUnregistered;

private:
	UPROPERTY()
	TScriptInterface<ISkillRepositoryInterface> SkillRepository;

	template<typename T>
	void ExecuteWithEvents(UE::Tasks::TTask<T> RepositoryTask, const FGuid& PlayerGuid, const FString& OperationName);
};
