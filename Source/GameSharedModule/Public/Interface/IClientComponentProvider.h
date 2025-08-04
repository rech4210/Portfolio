#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IClientComponentProvider.generated.h"

class USkillComponent;
struct FBossDataStruct;

UINTERFACE(MinimalAPI)
class UClientManagerInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IClientManagerInterface
{
	GENERATED_BODY()

public:
	virtual void RegistClientComponent(UActorComponent* Component) = 0;
	virtual void InitializeUI() = 0;
	virtual void ProcessRegistration(const FString& Username, const FString& Password) = 0;
	virtual void ProcessLogin(const FString& Username, const FString& Password) = 0;
	virtual void HandleRegistrationResult(bool bSuccess, const FString& Message) = 0;
	virtual void HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId) = 0;
	
	virtual void ProcessMouseOverDetection() = 0;
	virtual void NotifyStateChanged() = 0;
	virtual void ProcessBossData(const FBossDataStruct& BossData) = 0;
	
	virtual void SkillHUDReplication(const struct FSkillSlotReplicationArray& SkillSlotsReplication) = 0;
};


UINTERFACE(MinimalAPI)
class UClientAuthInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IClientAuthInterface
{
	GENERATED_BODY()

public:
	virtual void InitializeAuth() = 0;
	virtual TScriptInterface<IClientManagerInterface> GetClientSubSystem() = 0;
	virtual void RequestRegistration(const FString& Username, const FString& Password) = 0;
	virtual void RequestLogin(const FString& Username, const FString& Password) = 0;
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) = 0;
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) = 0;
};

UINTERFACE(MinimalAPI)
class UClientUIInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IClientUIInterface
{
	GENERATED_BODY()

public:
	virtual void InitializeUI() = 0;
	virtual void HandleMouseOverDetection() = 0;
	virtual void NotifyStateChanged() = 0;
	virtual void ReceiveBossData(const FBossDataStruct& BossData) = 0;
	virtual void ReceiveSkillReplicationData(const struct FSkillSlotReplicationArray& SkillSlotsReplication) = 0;
};
