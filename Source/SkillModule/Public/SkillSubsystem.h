#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Repositories/ISkillRepositoryInterface.h"
#include "Interface/PlayerIdentityInterface.h"
#include "SkillSubsystem.generated.h"

class USkillConfigRepository;
class USkillStateRepository;
class USkillRepository;
class USkillDomainService;
class USkillComponent;
class USkillDataAsset;
struct FSkillDomain;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillDataLoadCompleted, TScriptInterface<IPlayerIdentityInterface>, PlayerIdentity, USkillComponent*, SkillComponent);

UCLASS()
class SKILLMODULE_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<ISkillRepositoryInterface> GetSkillRepository() const;

	void SetSkillRepository(TScriptInterface<ISkillRepositoryInterface> Repository);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	USkillDomainService* GetDomainService();

	void RequestLoadPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	void RequestSavePlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	void RequestUpdateSkillSlot(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, USkillDataAsset* SkillData);

	void RequestUpdateSkillCooldown(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime);

	void RequestClearPlayerSkills(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId);

	void RequestSwapSkillSlots(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FString& UserId, int32 SlotIndexA, int32 SlotIndexB);

public:
	UPROPERTY(BlueprintAssignable)
	FOnSkillDataLoadCompleted OnSkillDataLoadCompleted;

private:
	UFUNCTION()
	void OnPlayerSkillsLoaded(const FGuid& PlayerGuid);

	UPROPERTY()
	TScriptInterface<ISkillRepositoryInterface> SkillRepositoryInterface;

	UPROPERTY()
	USkillRepository* DefaultSkillRepository;

	UPROPERTY()
	TObjectPtr<USkillDomainService> DomainService;
};
